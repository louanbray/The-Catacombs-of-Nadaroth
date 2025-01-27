#ifndef PLAYER_H
#define PLAYER_H

#include <assert.h>

#include "constants.h"
#include "inventory.h"

#define KEY_Z_LOW 122
#define KEY_Z_HIGH 90
#define KEY_Q_LOW 113
#define KEY_Q_HIGH 81
#define KEY_S_LOW 115
#define KEY_S_HIGH 83
#define KEY_D_LOW 100
#define KEY_D_HIGH 68
#define KEY_W_LOW 119
#define KEY_W_HIGH 87
#define KEY_1 49
#define KEY_2 50
#define KEY_3 51
#define KEY_4 52
#define KEY_5 53
#define KEY_6 54
#define KEY_7 55
#define KEY_8 56
#define KEY_9 57
#define KEY_ARROW_UP 65
#define KEY_ARROW_DOWN 66
#define KEY_ARROW_RIGHT 67
#define KEY_ARROW_LEFT 68

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