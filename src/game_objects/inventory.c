#include "inventory.h"

#include "../managers/achievements_manager.h"
#include "../managers/assets_manager.h"
#include "../managers/projectile_manager.h"
#include "item.h"

static int last_hotbar_index = 0;

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
    if (is_hotbar_full(h) || !e || !h) {
        return;
    }

    UsableItem type = get_item_usable_type(e);
    UsableItemAssetFile* uif = get_usable_item_file(type);

    Rarity class = uif->specs.specs[0];
    if (class == RARITY_NADINO) set_achievement_progress(ACH_SECRET_FINDER, 1);
    if (type < USABLE_ITEM_BOWS_END && type > USABLE_ITEM_NOT_USABLE) add_achievement_progress(ACH_CRAFTSMAN, 1);
    if (type > USABLE_ITEM_FOOD_START && type < USABLE_ITEM_COUNT) add_achievement_progress(ACH_GOURMET, 1);

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

void hotbar_drop(hotbar* h, int index, bool free_item_dropped) {
    if (h->items[index] == NULL) return;

    UsableItem type = get_item_usable_type(h->items[index]);
    if (type < USABLE_ITEM_BOWS_END && type > USABLE_ITEM_NOT_USABLE) add_achievement_progress(ACH_CRAFTSMAN, -1);
    if (type > USABLE_ITEM_FOOD_START && type < USABLE_ITEM_COUNT) add_achievement_progress(ACH_GOURMET, -1);

    if (free_item_dropped) free_item(h->items[index]);

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

void destroy_hotbar(hotbar* h) {
    if (h == NULL) return;
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        if (h->items[i] != NULL)
            free_item(h->items[i]);
    }
    free(h->items);
    free(h);
}

void bow_check_flag() {
    last_hotbar_index = -1;
}

int get_last_hotbar_index() {
    return last_hotbar_index;
}

void set_last_hotbar_index(int index) {
    last_hotbar_index = index;
}

int get_hotbar_index_of_usable_item(hotbar* h, UsableItem type) {
    if (!h) return -1;
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        if (h->items[i] != NULL)
            if (get_item_usable_type(h->items[i]) == type) return i;
    }
    return -1;
}