#ifndef PROJECTILE_MANAGER_H
#define PROJECTILE_MANAGER_H

#include <stdbool.h>

typedef struct Render_Buffer Render_Buffer;
typedef struct player player;
typedef struct item item;

/**
 * Spawn a player projectile toward the given target coordinates.
 *
 * The projectile system derives the projectile's damage, speed, range,
 * and infinity behavior from the player's currently selected weapon.
 * In hard difficulty, the player can only have one active projectile at a time.
 *
 * @param screen Active render buffer used to draw projectile updates.
 * @param p Player that fires the projectile.
 * @param target_x Target X coordinate in screen space.
 * @param target_y Target Y coordinate in screen space.
 */
void fire_projectile(Render_Buffer* screen, player* p, int target_x, int target_y);

/**
 * Start the projectile system thread and reset per-run projectile state.
 *
 * This also initializes the projectile RNG seed and records the current run
 * start time and enemy count.
 *
 * @param screen Active render buffer used by the projectile loop.
 * @param p Current player instance.
 * @param seed Seed used to drive projectile and enemy firing randomness.
 */
void init_projectile_system(Render_Buffer* screen, player* p, int seed);

/**
 * Deactivate all active projectiles and clear them from the screen when possible.
 *
 * @param screen Active render buffer used to inspect and clear projectile cells.
 */
void kill_all_projectiles(Render_Buffer* screen);

/**
 * Simulate the player's being hit by a projectile or enemy attack.
 *
 * This reuses the enemy-hit callback path to apply damage and trigger the same
 * death handling used by the projectile system.
 *
 * @param damage Damage amount to apply.
 * @param p Player receiving the hit.
 * @param screen Active render buffer used for any resulting redraws.
 */
void simulate_projectile_hit(int damage, player* p, Render_Buffer* screen);

/**
 * Add the number of enemies in the player's current chunk to the global enemy total.
 *
 * @param p Current player instance.
 */
void add_total_enemies(player* p);

/**
 * Reset the tracked global enemy count to zero.
 */
void reset_total_enemies();

/**
 * Stop the projectile thread if it is running and wait for it to exit.
 */
void stop_projectile_system();

/**
 * Restart the projectile system thread without reapplying run-start bookkeeping.
 *
 * @param screen Active render buffer used by the projectile loop.
 * @param p Current player instance.
 * @param seed Seed used to drive projectile and enemy firing randomness.
 */
void restart_projectile_system(Render_Buffer* screen, player* p, int seed);

#endif