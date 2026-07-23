#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include <stdbool.h>

#include "../game_objects/inventory.h"
#include "../game_objects/map.h"
#include "../game_objects/player.h"

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

/// @brief Saves a chunk to the persistent cache.
/// @details Serializes the chunk and stores it in the cache so it can be
/// unloaded from memory and restored later.
/// @param ck Pointer to the chunk to save.
/// @return true if the chunk was successfully saved, false otherwise.
bool save_chunk_to_cache(chunk* ck);

/// @brief Checks whether a chunk is present in the cache.
/// @param x Chunk x-coordinate.
/// @param y Chunk y-coordinate.
/// @return true if the chunk exists in the cache, false otherwise.
bool is_chunk_in_cache(int x, int y);

/// @brief Loads a chunk from the cache.
/// @details Deserializes the cached chunk, recreates its runtime structures,
/// and associates it with the specified map.
/// @param m Pointer to the destination map.
/// @param x Chunk x-coordinate.
/// @param y Chunk y-coordinate.
/// @return Pointer to the loaded chunk, or NULL if the chunk is not cached
/// or loading failed.
chunk* load_chunk_from_cache(map* m, int x, int y);

/// @brief Clears the entire chunk cache.
/// @details Removes all cached chunk data and releases any associated memory.
void clear_chunk_cache();

#endif
