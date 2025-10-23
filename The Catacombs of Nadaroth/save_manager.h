#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include <stdbool.h>

#include "inventory.h"
#include "map.h"
#include "player.h"

/// @brief Save the current game state to a file
/// @param filename path to save file
/// @param p player
/// @param m map
/// @param h hotbar
/// @return true if save was successful, false otherwise
bool save_game(const char* filename, player* p, map* m, hotbar* h);

/// @brief Load a game state from a file
/// @param filename path to save file
/// @param p player to load into
/// @param m map to load into
/// @param h hotbar to load into
/// @return true if load was successful, false otherwise
bool load_game(const char* filename, player* p, map* m, hotbar* h);

/// @brief Check if a save file exists
/// @param filename path to check
/// @return true if file exists, false otherwise
bool save_file_exists(const char* filename);

/// @brief Delete a save file
/// @param filename path to delete
/// @return true if deletion was successful, false otherwise
bool delete_save(const char* filename);

#endif
