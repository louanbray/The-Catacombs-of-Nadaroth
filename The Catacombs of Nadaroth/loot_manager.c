#include "loot_manager.h"

#include "dynarray.h"
#include "item.h"

dynarray** loot_manager = NULL;
const int loot_table_count = 4;

void create_loot_tables() {
    loot_manager = malloc(sizeof(dynarray*) * loot_table_count);
    for (int i = 0; i < loot_table_count; i++) {
        loot_manager[i] = create_dyn();
    }
}

void add_loot_to_loot_table(int display, UsableItem usable_type, int table_index) {
    if (table_index < 0 || table_index >= loot_table_count) {
        return;
    }

    append(loot_manager[table_index], generate_item(0, 0, PICKABLE, display, usable_type, -1));
}

item* generate_loot(lootable* loot) {
    int random_value = rand() % 100;
    int total_weight = loot->bronze + loot->silver + loot->gold + loot->nadino;

    int index = loot_table_count - 1 - (random_value < (loot->bronze + loot->silver + loot->gold) * 100 / total_weight) - (random_value < (loot->bronze + loot->silver) * 100 / total_weight) - (random_value < loot->bronze * 100 / total_weight);
    int length = len_dyn(loot_manager[index]);
    int random_index = rand() % length;
    item* loot_item = get_dyn(loot_manager[index], random_index);
    item* new_loot = generate_item(0, 0, PICKABLE, get_item_display(loot_item), get_item_usable_type(loot_item), -1);

    return new_loot;
}

void init_loot_tables() {
    create_loot_tables();

    // BRONZE LOOT
    add_loot_to_loot_table(66, BASIC_BOW, BRONZE);
    add_loot_to_loot_table(79, ONION_RING, BRONZE);
    // add_loot_to_loot_table(68, SCHOOL_DISHES, BRONZE);

    // SILVER LOOT
    add_loot_to_loot_table(66, ADVANCED_BOW, SILVER);
    add_loot_to_loot_table(83, STOCKFISH, SILVER);
    add_loot_to_loot_table(10055, BOMB, SILVER);

    // GOLD LOOT
    add_loot_to_loot_table(66, SUPER_BOW, GOLD);
    add_loot_to_loot_table(71, GOLDEN_APPLE, GOLD);

    // NADINO LOOT
    add_loot_to_loot_table(66, NADINO_BOW, NADINO);
}