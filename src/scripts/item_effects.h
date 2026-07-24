#ifndef ITEM_EFFECTS_H
#define ITEM_EFFECTS_H

#include <stdbool.h>

#include "../utils/constants.h"

typedef struct player player;

/**
 * @brief Triggers the callback linked to the use of a UsableItem
 *
 * @param type The UsableItem to use
 * @param player The player that will be affected
 * @return true if the item needs to be destroyed else false
 */
bool use_item(UsableItem type, player* p);

#endif  // ITEM_EFFECTS_H