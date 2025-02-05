#ifndef PLAYER_H
#define PLAYER_H

#include <assert.h>

#include "constants.h"
#include "inventory.h"

/// @brief Map
typedef struct map map;
/// @brief Chunk
typedef struct chunk chunk;
/// @brief Hotbar
typedef struct hotbar hotbar;
/// @brief Player
typedef struct player player;

/// @brief Return a player linked to the map spawn chunk
/// @param map map
/// @return player
player* create_player(map* map);

/// @brief Return player coord x in chunk
/// @param player player
/// @return x
int get_player_x(player* player);

/// @brief Return player coord y in chunk
/// @param player player
/// @return y
int get_player_y(player* player);

/// @brief Return player previous coord x in chunk
/// @param player player
/// @return previous x
int get_player_px(player* player);

/// @brief Return player previous coord y in chunk
/// @param player player
/// @return previous y
int get_player_py(player* player);

/// @brief Return player current chunk
/// @param player player
/// @return chunk*
chunk* get_player_chunk(player* player);

/// @brief Return player hotbar
/// @param player player
/// @return hotbar
hotbar* get_player_hotbar(player* player);

/// @brief Return player design (char)
/// @param player player
/// @return char design
int get_player_design(player* player);

/// @brief Return player name
/// @param player player
/// @return name
char* get_player_name(player* player);

/// @brief Return player health
/// @param player player
/// @return health
int get_player_health(player* player);

/// @brief Return player max health
/// @param player player
/// @return health
int get_player_max_health(player* player);

/// @brief Link a hotbar to the player
/// @param player player
/// @param hotbar
void link_hotbar(player* player, hotbar* hotbar);

/// @brief Move player of 1 unit in a chunk following the direction
/// @param player player
/// @param dir Direction
/// @return refresh type
int move_player(player* player, Direction dir);

/// @brief Move player to a new chunk following a direction/way
/// @param player player
/// @param dir Direction/Type
void move_player_chunk(player* player, Direction dir);

/// @brief Damage the player health and set to 0 if dead
/// @param player player
/// @param damage damage (>=0) else use heal
void damage_player(player* player, int damage);

/// @brief Heal the player and set to max if too high
/// @param player player
/// @param damage heal (>=0) else use heal
void heal_player(player* player, int heal);

/// @brief Change the player max health
/// @param player player
/// @param health new health
void set_player_max_health(player* player, unsigned int health);

#endif