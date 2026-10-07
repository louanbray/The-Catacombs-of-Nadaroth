#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#include "../game_objects/inventory.h"
#include "../game_objects/map.h"
#include "../game_objects/player.h"
#include "../utils/game_status.h"

/// @brief metadata for a save slot, stored in a separate .meta file.
/// Read by the UI to display save informations wthout openning the .dat
typedef struct SaveSlotMeta {
    uint32_t magic;
    int score;
    GamePhase phase;
    Difficulty difficulty;
    int mental_health;
    PlayerClass player_class;
    int64_t time_played_sec;
    int64_t saved_at_sec;
} SaveSlotMeta;

#define SAVE_SLOTS 7

/// @brief Save the current game state to a file
/// @param slot slot to save to
/// @param p player
/// @param m map
/// @param h hotbar
/// @return true if save was successful, false otherwise
bool save_game(int slot, player* p, map* m, hotbar* h);

/// @brief Load a game state from a file
/// @param slot slot to load from
/// @param p player to load into
/// @param m map to load into
/// @param h hotbar to load into
/// @return true if load was successful, false otherwise
bool load_game(int slot, player* p, map* m, hotbar* h);

/// @brief Check if a save file exists
/// @param filename path to check
/// @return true if file exists, false otherwise
bool save_file_exists(const char* filename);

/// @brief Delete a save file
/// @param filename path to delete
/// @return true if deletion was successful, false otherwise
bool delete_save(const char* filename);

/// @brief Saves a chunk to the single binary cache file.
/// @param m Pointer to map.
/// @param ck Pointer to the chunk to save.
/// @return true if the chunk was successfully saved, false otherwise.
bool save_chunk_to_cache(map* m, chunk* ck);

/// @brief Checks whether a chunk is present in the cache.
/// @param m Pointer to map.
/// @param x Chunk x-coordinate.
/// @param y Chunk y-coordinate.
/// @return true if the chunk exists in the cache, false otherwise.
bool is_chunk_in_cache(map* m, int x, int y);

/// @brief Loads a chunk from the single binary cache file.
/// @param m Pointer to the destination map.
/// @param x Chunk x-coordinate.
/// @param y Chunk y-coordinate.
/// @return Pointer to the loaded chunk, or NULL on failure.
chunk* load_chunk_from_cache(map* m, int x, int y);

/// @brief Clears the single binary chunk cache file.
void clear_chunk_cache();

/// @brief Gives the state of the save at this slot
/// @param slot save slot
/// @return 0 if save file doesn't exist, 1 if it does, 2 if it is the save the current game is based on
int get_save_slot_state(int slot);

int get_current_save_slot();

/// @brief Load the lightweight metadata for a save slot.
/// @param slot Save slot index.
/// @param out  Pointer to a SaveSlotMeta struct to fill.
/// @return true if the .meta file was read successfully, false otherwise.
bool load_slot_meta(int slot, SaveSlotMeta* out);

#endif
