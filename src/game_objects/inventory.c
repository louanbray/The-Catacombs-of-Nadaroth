#include "inventory.h"

#include "../managers/achievements_manager.h"
#include "../managers/assets_manager.h"
#include "../managers/projectile_manager.h"
#include "item.h"
#include "player.h"

static int last_hotbar_index = 0;

/// @brief Hotbar
typedef struct hotbar {
    item** items;
    int selected, entries;
    item* selected_item;
    int max_size;
} hotbar;

/// @brief Keyholder
typedef struct keyholder {
    KeyHolderLevel level;
    int* key_nb;
} keyholder;

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

keyholder* create_keyholder() {
    keyholder* k = malloc(sizeof(keyholder));
    k->level = KEYHOLDER_LOCKED;
    k->key_nb = calloc(RARITY_COUNT, sizeof(int));
    return k;
}

Rarity get_key_rarity(UsableItem key) {
    if (key <= USABLE_ITEM_BOWS_END || key >= USABLE_ITEM_KEYS_END) return -1;
    UsableItemAssetFile* uif = get_usable_item_file(key);
    return uif->specs.specs[0];
}

void set_hotbar(hotbar* h, int index, item* e) {
    if (!h || index < 0 || index >= HOTBAR_SIZE) return;

    if (h->items[index] != NULL) {
        h->entries--;
    }

    h->items[index] = e;

    if (e != NULL) {
        h->entries++;
    }

    if (h->selected == index) {
        h->selected_item = e;
    }
}

void clear_hotbar(hotbar* h) {
    if (!h) return;

    for (int i = 0; i < HOTBAR_SIZE; i++) {
        if (h->items[i] != NULL) {
            free_item(h->items[i]);
            h->items[i] = NULL;
        }
    }

    h->selected = 0;
    h->entries = 0;
    h->selected_item = NULL;
    bow_check_flag();
}

void set_keyholder_keys_of_rarity(keyholder* k, Rarity rarity, int nb) {
    if (!k || !k->key_nb || nb < 0 || rarity >= RARITY_COUNT) return;
    k->key_nb[rarity] = nb;
}

void add_keyholder_keys_of_rarity(keyholder* k, Rarity rarity, int nb) {
    if (!k || !k->key_nb || rarity >= RARITY_COUNT) return;
    int new_nb = k->key_nb[rarity] + nb;
    k->key_nb[rarity] = new_nb > 0 ? new_nb : 0;
}

void set_keyholder_level(keyholder* k, int level) {
    if (!k) return;
    if (level > KEYHOLDER_MAX_LEVEL) level = KEYHOLDER_MAX_LEVEL - 1;
    if (level < 0) level = KEYHOLDER_LOCKED;
    k->level = level;
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

static bool can_keyholder_handle_rarity(keyholder* k, Rarity rarity) {
    return (int)k->level > (int)rarity;
}

int get_keyholder_keys_of_rarity(keyholder* k, Rarity rarity) {
    if (!k || !k->key_nb || rarity >= RARITY_COUNT) return 0;
    if (!can_keyholder_handle_rarity(k, rarity)) return 0;
    return k->key_nb[rarity];
}

KeyHolderLevel get_keyholder_level(keyholder* k) {
    return k->level;
}

bool keyholder_has_key_of_rarity(keyholder* k, Rarity rarity) {
    if (!k || !k->key_nb || rarity >= RARITY_COUNT) return 0;
    if (!can_keyholder_handle_rarity(k, rarity)) return 0;
    return k->key_nb[rarity] > 0;
}

void keyholder_level_up(keyholder* k) {
    if (!k) return;
    if (k->level + 1 < KEYHOLDER_MAX_LEVEL) k->level++;
}

bool is_hotbar_full(hotbar* h) {
    return h->entries == HOTBAR_SIZE;
}

void pickup(player* p, item* e) {
    hotbar* h = get_player_hotbar(p);
    keyholder* k = get_player_keyholder(p);
    if (is_hotbar_full(h) || !e || !h) {
        return;
    }

    UsableItem type = get_item_usable_type(e);
    UsableItemAssetFile* uif = get_usable_item_file(type);

    Rarity class = uif->specs.specs[0];
    if (class == RARITY_NADINO) set_achievement_progress(ACH_SECRET_FINDER, 1);
    if (type < USABLE_ITEM_BOWS_END && type > USABLE_ITEM_NOT_USABLE) add_achievement_progress(ACH_CRAFTSMAN, 1);
    if (type > USABLE_ITEM_FOOD_START && type < USABLE_ITEM_COUNT) add_achievement_progress(ACH_GOURMET, 1);
    if (type > USABLE_ITEM_BOWS_END && type < USABLE_ITEM_KEYS_END && can_keyholder_handle_rarity(k, class)) {
        add_keyholder_keys_of_rarity(k, class, 1);
        free_item(e);
        return;
    }

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

void destroy_keyholder(keyholder* k) {
    if (k == NULL) return;
    if (k->key_nb) free(k->key_nb);
    free(k);
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