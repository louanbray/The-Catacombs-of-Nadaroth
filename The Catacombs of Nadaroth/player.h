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

/// @brief Returns a player linked to the map spawn chunk
/// @param map map
/// @return player
player* create_player(map* map);

/// @brief Returns player coord x in chunk
/// @param player player
/// @return x
int get_player_x(player* player);

/// @brief Returns player coord y in chunk
/// @param player player
/// @return y
int get_player_y(player* player);

/// @brief Returns player previous coord x in chunk
/// @param player player
/// @return previous x
int get_player_px(player* player);

/// @brief Returns player previous coord y in chunk
/// @param player player
/// @return previous y
int get_player_py(player* player);

/// @brief Returns player current chunk
/// @param player player
/// @return chunk*
chunk* get_player_chunk(player* player);

/// @brief Returns player hotbar
/// @param player player
/// @return hotbar
hotbar* get_player_hotbar(player* player);

/// @brief Returns player design (char)
/// @param player player
/// @return char design
int get_player_design(player* player);

/// @brief Returns player name
/// @param player player
/// @return name
char* get_player_name(player* player);

/// @brief Returns player current map
/// @param p player
/// @return map
map* get_player_map(player* p);

/// @brief Returns player health
/// @param player player
/// @return health
int get_player_health(player* player);

/// @brief Returns player max health
/// @param player player
/// @return health
int get_player_max_health(player* player);

/// @brief Returns player damage
/// @param player player
/// @return damage
int get_player_damage(player* player);

/// @brief Set player damage
/// @param player player
/// @param damage damage
void set_player_damage(player* player, unsigned int damage);

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
/// @return true if dead else false
/// @note This function will set the player's health to 0 if the damage exceeds the current health.
bool damage_player(player* player, int damage);

/// @brief Heal the player and set to max if too high
/// @param player player
/// @param damage heal (>=0) else use heal
void heal_player(player* player, int heal);

/// @brief Change the player max health
/// @param player player
/// @param health new health
void set_player_max_health(player* player, unsigned int health);

/// @brief Destroy the player closest chunk
/// @param p player
void destroy_player_cchunk(player* p);

/**
 * @brief Checks if the player has infinite range enabled.
 *
 * @param p Pointer to the player structure.
 * @return true if the player has infinite range, false otherwise.
 */
int get_player_range(player* p);

/**
 * @brief Retrieves the score of the player.
 *
 * @param p Pointer to the player structure.
 * @return The current score of the player.
 */
int get_player_score(player* p);

/**
 * @brief Sets whether the player has infinite range enabled.
 *
 * @param p Pointer to the player structure.
 * @param infinite Boolean value indicating whether to enable or disable infinite range.
 */
void set_player_range(player* p, int range);

/**
 * @brief Sets the score of the player.
 *
 * @param p Pointer to the player structure.
 * @param score The new score to assign to the player.
 */
void set_player_score(player* p, int score);

/**
 * @brief Adds a specified value to the player's current score.
 *
 * @param p Pointer to the player structure.
 * @param score The value to add to the player's current score.
 */
void add_player_score(player* p, int score);

/**
 * @brief Retrieves the arrow speed of a player.
 *
 * @param p Pointer to the player structure.
 * @return The current arrow speed of the player.
 */
int get_player_arrow_speed(player* p);

/**
 * @brief Sets the arrow speed for a player.
 *
 * @param p Pointer to the player structure.
 * @param speed The new arrow speed to set for the player.
 */
void set_player_arrow_speed(player* p, int speed);

/**
 * @brief Handles the event of a player's death
 *
 * @param p Pointer to the player
 */
void player_death(player* p);

/**
 * @brief Returns the number of deaths of a player
 *
 * @param p Pointer to the player
 * @return The death count of the player
 */
int get_player_deaths(player* p);

/**
 * @brief Returns the mental health value of a player
 *
 * @param p Pointer to the player
 * @return The mental health value of the player
 */
int get_player_mental_health(player* p);

/**
 * @brief Retrieve the color associated with a player.
 *
 * @param p Pointer to the player whose color will be returned.
 * @return An integer encoding the player's color. The concrete mapping of
 *         integer values to colors is defined by the project's color
 *         constants (e.g., COLOR_RED, COLOR_BLUE). Behavior is undefined if
 *         p is NULL.
 */
Color get_player_color(player* p);

bool has_infinity(player* p);

/**
 * @brief Increments the death count of a player
 *
 * @param p Pointer to the player
 */
void add_player_deaths(player* p);

/**
 * @brief Sets the mental health of a player to a specified value
 *
 * @param p Pointer to the player
 * @param mental_health The new mental health value
 */
void set_player_mental_health(player* p, int mental_health);

/**
 * @brief Modifies the mental health of a player by adding the specified value
 *        and ensures it stays within the range [0, 4]
 *
 * @param p Pointer to the player
 * @param mental_health The value to add to the player's mental health
 */
void modify_player_mental_health(player* p, int mental_health);

/**
 * @brief Gets the current game phase of the player
 *
 * @param p Pointer to the player object
 * @return GamePhase The current phase of the player
 */
GamePhase get_player_phase(player* p);

/**
 * @brief Sets the game phase of the player
 *
 * @param p Pointer to the player object
 * @param phase The GamePhase to set for the player
 */
void set_player_phase(player* p, GamePhase phase);

/**
 * @brief Set the visual color of a player.
 *
 * @param p Pointer to the player to modify. Must be a valid, mutable pointer
 * @param color Color value to assign to the player.
 */
void set_player_color(player* p, Color color);

void set_player_design(player* p, int design);

void set_player_class(player* p, int class);

void set_player_infinity(player* p, bool infinite);

/**
 * @brief Increments the player's game phase to the next phase
 *
 * @param p Pointer to the player object
 */
void increment_player_phase(player* p);

#endif