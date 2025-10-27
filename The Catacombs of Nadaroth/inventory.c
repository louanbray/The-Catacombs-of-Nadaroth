#include "inventory.h"

#include "achievements.h"
#include "assets_manager.h"
#include "item.h"
#include "projectile.h"

/// @brief Hotbar
typedef struct hotbar {
    item** items;
    int selected, entries;
    item* selected_item;
    int max_size;
} hotbar;

hotbar* create_hotbar() {
    hotbar* h = malloc(sizeof(hotbar));
    item** i = calloc(9, sizeof(item*));
    h->items = i;
    h->selected = 0;
    h->entries = 0;
    h->selected_item = NULL;
    h->max_size = HOTBAR_SIZE;
    return h;
}

void set_hotbar(hotbar* h, int index, item* e) {
    h->items[index] = e;
}

item* get_hotbar(hotbar* h, int index) {
    return h->items[index];
}

int get_selected_slot(hotbar* h) {
    return h->selected;
}

item* get_selected_item(hotbar* h) {
    return h->selected_item;
}

int get_hotbar_max_size(hotbar* h) {
    return h->max_size;
}

bool is_hotbar_full(hotbar* h) {
    return h->entries == HOTBAR_SIZE;
}

void pickup(hotbar* h, item* e) {
    if (is_hotbar_full(h)) {
        return;
    }

    UsableItemAssetFile* uif = get_usable_item_file(get_item_usable_type(e));
    Rarity class = uif->specs.specs[0];
    if (class == NADINO) set_achievement_progress(ACH_SECRET_FINDER, 1);
    if (get_item_usable_type(e) < BOWS_END) add_achievement_progress(ACH_CRAFTSMAN, 1);
    if (get_item_usable_type(e) > FOOD_START) add_achievement_progress(ACH_GOURMET, 1);

    int i = 0;

    for (i = 0; i < HOTBAR_SIZE; i++) {
        if (h->items[i] == NULL) {
            break;
        }
    }

    h->items[i] = e;

    if (i == h->selected) {
        h->selected_item = e;
        bow_check_flag();
    }

    h->entries += 1;
}

void drop(hotbar* h, int index) {
    if (h->items[index] == NULL) return;

    free_item(h->items[index]);
    if (get_item_usable_type(h->items[index]) < BOWS_END) add_achievement_progress(ACH_CRAFTSMAN, -1);
    if (get_item_usable_type(h->items[index]) > FOOD_START) add_achievement_progress(ACH_GOURMET, -1);
    h->items[index] = NULL;

    if (h->selected == index) {
        h->selected_item = NULL;
        bow_check_flag();
    }

    h->entries += -1;
}

void select_slot(hotbar* h, int index) {
    if (index != h->selected) {
        h->selected = index;
        h->selected_item = get_hotbar(h, index);
    }
}

int get_hotbar_entries(hotbar* h) {
    return h->entries;
}