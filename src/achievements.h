#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

#include <stdbool.h>

enum AchievementID {
    ACH_FIRST_BLOOD,       // OK
    ACH_MONSTER_HUNTER,    // OK
    ACH_TREASURE_SEEKER,   // OK
    ACH_UNSTOPPABLE,       // OK
    ACH_MASTER_EXPLORER,   // OK
    ACH_DAWN_BREAKER,      // OK
    ACH_UNSHAKEN,          // OK
    ACH_SPEED_RUNNER,      // OK
    ACH_SURVIVOR,          // OK
    ACH_LOOT_COLLECTOR,    // OK
    ACH_SECRET_FINDER,     // OK
    ACH_BOSS_SLAYER,       // OK
    ACH_PERFECTIONIST,     // NOK
    ACH_CRAFTSMAN,         // OK
    ACH_GOURMET,           // OK
    ACH_HERO_OF_NADAROTH,  // OK
    ACHIEVEMENT_COUNT,
};

/**
 * @brief Returns an achievement points reward upon completion
 *
 * @param id AchievementID
 * @return Achievement points reward
 */
int get_achievement_points(enum AchievementID id);

/**
 * @brief Returns the player's progress on an achievement
 *
 * @param id AchievementID
 * @return Achievement progress
 */
int get_achievement_progress(enum AchievementID id);

/**
 * @brief Returns the progress needed for an achievement to be completed
 *
 * @param id AchievementID
 * @return Achievement max progress
 */
int get_achievement_max_progress(enum AchievementID id);

/**
 * @brief Returns the sum of all achievements points rewards
 * @return Points sum
 */
int get_total_points();

/**
 * @brief Returns the number of completed achievements (progress=max progress)
 * @return Number of completed achievements
 */
int get_completed_achiemevents();

/**
 * @brief Returns an achievement's name
 *
 * @param id AchievementID
 * @return Achievement's name
 */
const char* get_achievement_name(enum AchievementID id);

/**
 * @brief Returns an achievement's description
 *
 * @param id AchievementID
 * @return Achievement's description
 */
const char* get_achievement_description(enum AchievementID id);

/**
 * @brief Set an achievement progress to a specific value
 *
 * @param id AchievementID
 * @param progress New achievement progress  (if OOB will fix to the nearest value in bounds)
 */
void set_achievement_progress(enum AchievementID id, int progress);

/**
 * @brief Add progress to an achievement
 *
 * @param id AchievementID
 * @param progress Progress to add (if the new progress OOB will fix to the nearest value in bounds)
 */
void add_achievement_progress(enum AchievementID id, int progress);

/**
 * @brief Returns if an achievement is unlocked (progress == max_progress)
 *
 * @param id AchievementID
 * @return Is unlocked?
 */
bool is_achievement_unlocked(enum AchievementID id);

/**
 * @brief Some achievements are based on the current run (survivor, craftsman).
 * This function resets them to their default value (0).
 */
void reset_run_based_achievements();

/**
 * @brief This functions load the player's progress on achievements from the local file.
 * It also resets run based achievements
 */
void load_achievements();

/**
 * @brief This function will save the achievements progress to the local file.
 */
void save_achievements();

#endif