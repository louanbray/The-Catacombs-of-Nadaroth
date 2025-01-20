
#ifndef MAP_H
#define MAP_H

#include "generation.h"

/// @brief Map
typedef struct map map;
/// @brief Player
typedef struct player player;

/// @brief Create a map, a hashmap of chunk* with spawn value
/// @return map
map* create_map();

/// @brief Get the chunk in x,y or create it
/// @param m map
/// @param pos_x chunk x
/// @param pos_y chunk y
/// @return accessed or created chunk
chunk* get_chunk(map* map, int pos_x, int pos_y);

/// @brief Get the loaded chunk when passing through a certain gate of current chunk
/// @param map map
/// @param current_chunk current chunk
/// @param dir gate orientation / type
/// @return created chunk or accessed chunk
chunk* get_chunk_from(map* map, chunk* current_chunk, enum Direction dir);

/// @brief Get linked spawn chunk of a map
/// @param map map
/// @return spawn chunk
chunk* get_spawn(map* map);

/// @brief Return linked player
/// @param map map
/// @return player
player* get_player(map* map);

/// @brief Link a player to a map
/// @param map map
/// @param player player
void set_map_player(map* map, player* player);

/// @brief Free full chunk in the map and itself
/// @param map map
/// @param chunk chunk to free
void destroy_chunk(map* map, chunk* chunk);

/// @brief Print the chunk with his coords, pointer, the link pointer and the linked chunks
/// @param chunk chunk to lookup
void print_chunk(chunk* chunk);

/// @brief Print the @map->hashtable and the @map->spawn chunk
/// @param map map
void print_map(map* map);

#endif
