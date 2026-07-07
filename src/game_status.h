#ifndef GAME_STATUS_H
#define GAME_STATUS_H

#include <pthread.h>
#include <sys/time.h>

extern pthread_mutex_t pause_mutex;
extern pthread_cond_t pause_cond;
extern int GAME_PAUSED;

typedef enum Difficulty {
    DIFFICULTY_EASY,
    DIFFICULTY_HARD,
} Difficulty;

/**
 * Pauses the game by increasing GAME_PAUSED (stackable)
 */
void pause_game(void);

/**
 * Resumes the game by decreasing GAME_PAUSED (stackable).
 * GAME_PAUSED cannot be inferior to 0.
 */
void resume_game(void);

/**
 * Enables the debug mode
 */
void set_debug_mode(int mode);

/**
 * Is the debug mode enabled?
 */
int is_debug_mode(void);

/**
 * Is a reset needed?
 */
int need_reset(void);

/**
 * @brief Set the game started time to the value specified
 *
 * @param started timeval, when the game was started
 */
void set_game_started(struct timeval started);

/**
 * @brief Get the time when the game was started
 */
struct timeval get_game_started(void);

/**
 * @brief Get the time elapsed since the game started
 */
struct timeval get_time_played(void);

/**
 * @brief Add time to the total time played
 *
 * @param delta timeval, will get added to the total time played
 */
void add_time_played(struct timeval delta);

/**
 * @brief Set the total time played to a specific valu
 */
void set_time_played(struct timeval time_played);

/**
 * @brief Changes the difficulty of the game
 */
void set_difficulty(Difficulty difficulty);

/**
 * @brief Get the game's current difficulty
 */
Difficulty get_difficulty();

#endif