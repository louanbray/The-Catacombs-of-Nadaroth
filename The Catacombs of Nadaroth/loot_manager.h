#ifndef LOOT_MANAGER_H
#define LOOT_MANAGER_H

typedef struct item item;
typedef struct lootable lootable;

item* generate_loot(lootable* loot);

void init_loot_tables();

#endif