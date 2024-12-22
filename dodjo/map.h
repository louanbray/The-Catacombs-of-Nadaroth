
#ifndef MAP_H
#define MAP_H

#include "generation.h"

/// @brief Map
typedef struct map map;
/// @brief Player
typedef struct player player;

/// @brief Create a map, a hashmap of chunk* with spawn value
/// @param free_fun Free elements function
/// @return map
map* create_map();

/// @brief Get the chunk in x,y or create it
/// @param m map
/// @param x chunk x
/// @param y chunk y
/// @return accessed or created chunk
chunk* get_chunk(map* m, int x, int y);

/// @brief Get the loaded chunk when passing through a certain gate of current chunk
/// @param m map
/// @param c1 current chunk
/// @param dir gate orientation / type
/// @return created chunk or accessed chunk
chunk* get_chunk_from(map* m, chunk* c1, int dir);

/// @brief Get linked spawn chunk of a map
/// @param m map
/// @return spawn chunk
chunk* get_spawn(map* m);

/// @brief Return linked player
/// @param m map
/// @return player
player* get_player(map* m);

/// @brief Link a player to a map
/// @param m map
/// @param p player
void set_map_player(map* m, player* p);

/// @brief Free full chunk in the map and itself
/// @param m map
/// @param ck chunk to free
void destroy_chunk(map* m, chunk* ck);

/// @brief Print the chunk with his coords, pointer, the link pointer and the linked chunks
/// @param ck chunk to lookup
void print_chunk(chunk* ck);

/// @brief Print the @map->hashtable and the @map->spawn chunk
/// @param m map
void print_map(map* m);

#endif
