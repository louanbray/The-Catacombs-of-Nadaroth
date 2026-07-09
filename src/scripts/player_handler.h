#ifndef PLAYER_HANDLER_H
#define PLAYER_HANDLER_H

#include "../game_objects/player.h"

/// @brief Handle the interactions between the player and the items on the ground
/// @param p player
/// @param x player next x
/// @param y player next y
/// @return true if can move else return false
PlayerMovementResult handle(player* player, int x, int y);

/// @brief Check if the player has interacted with a lootable while in range with a key in inventory
/// @param p player
/// @param x click x
/// @param y click y
/// @return bool : has a chest been opened?
bool check_lootable_interaction(player* p, int x, int y);

#endif