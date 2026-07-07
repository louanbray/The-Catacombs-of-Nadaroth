#ifndef LOOT_MANAGER_H
#define LOOT_MANAGER_H

typedef struct item item;
typedef struct lootable lootable;

/**
 * @brief Returns an item generated using the loot table odds from this specific lootable
 *
 * @param loot Pointer to a lootbable containing the odds of generating an item of said rarity.
 * Item have an uniform distribution inside their own rarity.
 *
 * @return New item copied from the loot tabe template.
 */
item* generate_loot(lootable* loot);

/// @brief Needed to use the loot tables as it adds the loot to the loot table.
void init_loot_tables();

#endif