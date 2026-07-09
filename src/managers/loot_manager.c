#include "loot_manager.h"

#include "../game_objects/item.h"
#include "../utils/dynarray.h"

dynarray** loot_manager = NULL;
static unsigned int loot_seed = 1;

Color get_color_for_rarity(Rarity rarity_index) {
    switch (rarity_index) {
        case RARITY_BRONZE:
            return COLOR_RED;
        case RARITY_SILVER:
            return COLOR_CYAN;
        case RARITY_GOLD:
            return COLOR_YELLOW;
        case RARITY_NADINO:
            return COLOR_MAGENTA;
        default:
            return COLOR_DEFAULT;
    }
}

void create_loot_tables() {
    loot_manager = malloc(sizeof(dynarray*) * LOOT_TABLE_COUNT);
    for (int i = 0; i < LOOT_TABLE_COUNT; i++) {
        loot_manager[i] = create_dyn();
    }
}

void seed_loot_manager(unsigned int seed) {
    loot_seed = seed;
}

static int loot_rand() {
    loot_seed = loot_seed * 1103515245 + 12345;
    return (unsigned int)(loot_seed / 65536) % 32768;
}

void add_loot_to_loot_table(int display, UsableItem usable_type, LootTableID table_index, Rarity rarity) {
    if (table_index < 0 || (int)table_index >= LOOT_TABLE_COUNT) {
        return;
    }
    item* loot_item = generate_item(0, 0, ITEMTYPE_PICKABLE, display, usable_type, -1);
    set_item_color(loot_item, get_color_for_rarity(rarity));
    append(loot_manager[table_index], loot_item);
}

item* generate_loot(lootable* loot) {
    if (loot == NULL || loot->id < 0 || (int)loot->id >= LOOT_TABLE_COUNT) {
        return NULL;
    }

    int total_weight = loot->none + loot->bronze + loot->silver + loot->gold + loot->nadino;
    if (total_weight <= 0) {
        return NULL;
    }

    int random_value = loot_rand() % total_weight;
    Rarity rolled_rarity;

    if (random_value < loot->none) {
        return NULL;
    } else if (random_value < loot->none + loot->bronze) {
        rolled_rarity = RARITY_BRONZE;
    } else if (random_value < loot->none + loot->bronze + loot->silver) {
        rolled_rarity = RARITY_SILVER;
    } else if (random_value < loot->none + loot->bronze + loot->silver + loot->gold) {
        rolled_rarity = RARITY_GOLD;
    } else {
        rolled_rarity = RARITY_NADINO;
    }

    dynarray* table = loot_manager[loot->id];
    int length = len_dyn(table);
    if (length == 0) {
        return NULL;
    }

    Color target_color = get_color_for_rarity(rolled_rarity);

    int match_count = 0;
    for (int i = 0; i < length; i++) {
        item* it = get_dyn(table, i);
        if (get_item_color(it) == target_color) {
            match_count++;
        }
    }

    item* loot_item = NULL;
    if (match_count > 0) {
        int random_match_index = loot_rand() % match_count;
        int current_match = 0;
        for (int i = 0; i < length; i++) {
            item* it = get_dyn(table, i);
            if (get_item_color(it) == target_color) {
                if (current_match == random_match_index) {
                    loot_item = it;
                    break;
                }
                current_match++;
            }
        }
    } else {
        loot_item = get_dyn(table, loot_rand() % length);
    }

    item* new_loot = generate_item(0, 0, ITEMTYPE_PICKABLE, get_item_display(loot_item), get_item_usable_type(loot_item), -1);
    set_item_color(new_loot, get_item_color(loot_item));

    return new_loot;
}

void init_loot_tables() {
    create_loot_tables();

    // BRONZE LOOT
    add_loot_to_loot_table(L'B', USABLE_ITEM_BASIC_BOW, LOOT_TABLE_CHEST, RARITY_BRONZE);
    add_loot_to_loot_table(L'O', USABLE_ITEM_ONION_RING, LOOT_TABLE_CHEST, RARITY_BRONZE);

    // SILVER LOOT
    add_loot_to_loot_table(L'B', USABLE_ITEM_ADVANCED_BOW, LOOT_TABLE_CHEST, RARITY_SILVER);
    add_loot_to_loot_table(L'S', USABLE_ITEM_STOCKFISH, LOOT_TABLE_CHEST, RARITY_SILVER);
    add_loot_to_loot_table(L'✧', USABLE_ITEM_BOMB, LOOT_TABLE_CHEST, RARITY_SILVER);

    // GOLD LOOT
    add_loot_to_loot_table(L'B', USABLE_ITEM_SUPER_BOW, LOOT_TABLE_CHEST, RARITY_GOLD);
    add_loot_to_loot_table(L'G', USABLE_ITEM_GOLDEN_APPLE, LOOT_TABLE_CHEST, RARITY_GOLD);
    // add_loot_to_loot_table(68, SCHOOL_DISHES, RARITY_GOLD);

    // NADINO LOOT
    // add_loot_to_loot_table(70, FORGOTTEN_DISH, RARITY_NADINO);
    add_loot_to_loot_table(L'B', USABLE_ITEM_NADINO_BOW, LOOT_TABLE_CHEST, RARITY_NADINO);

    // Keys
    add_loot_to_loot_table(CHEST_KEY_DESIGN, USABLE_ITEM_BRONZE_KEY, LOOT_TABLE_ENEMY_KEY, RARITY_BRONZE);
    add_loot_to_loot_table(CHEST_KEY_DESIGN, USABLE_ITEM_SILVER_KEY, LOOT_TABLE_ENEMY_KEY, RARITY_SILVER);
    add_loot_to_loot_table(CHEST_KEY_DESIGN, USABLE_ITEM_GOLD_KEY, LOOT_TABLE_ENEMY_KEY, RARITY_GOLD);
    add_loot_to_loot_table(CHEST_KEY_DESIGN, USABLE_ITEM_NADINO_KEY, LOOT_TABLE_ENEMY_KEY, RARITY_NADINO);
}