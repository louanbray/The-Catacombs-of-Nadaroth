#ifndef PLAYER_H
#define PLAYER_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#define KEY_Z_LOW 122
#define KEY_Z_HIGH 90
#define KEY_Q_LOW 113
#define KEY_Q_HIGH 81
#define KEY_S_LOW 115
#define KEY_S_HIGH 83
#define KEY_D_LOW 100
#define KEY_D_HIGH 68
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
/// @param p player
/// @return x
int get_player_x(player* p);

/// @brief Return player coord y in chunk
/// @param p player
/// @return y
int get_player_y(player* p);

/// @brief Return player previous coord x in chunk
/// @param p player
/// @return previous x
int get_player_px(player* p);

/// @brief Return player previous coord y in chunk
/// @param p player
/// @return previous y
int get_player_py(player* p);

/// @brief Return player current chunk
/// @param p player
/// @return chunk*
chunk* get_player_chunk(player* p);

/// @brief Return player hotbar
/// @param p player
/// @return hotbar
hotbar* get_player_hotbar(player* p);

/// @brief Return player design (char)
/// @param p player
/// @return char design
int get_player_design(player* p);

/// @brief Return player name
/// @param p player
/// @return name
char* get_player_name(player* p);

/// @brief Return player health
/// @param p player
/// @return health
int get_player_health(player* p);

/// @brief Link a hotbar to the player
/// @param p player
/// @param h
void link_hotbar(player* p, hotbar* h);

/// @brief Move player of 1 unit in a chunk following the direction
/// @param p player
/// @param dir Direction
/// @return refresh type
int move_player(player* p, int dir);

/// @brief Move player to a new chunk following a direction/way
/// @param p player
/// @param dir Direction/Type
void move_player_chunk(player* p, int dir);

/// @brief Damage the player health and set to 0 if dead
/// @param p player
/// @param damage damage (>=0)
void damage_player(player* p, int damage);

#endif