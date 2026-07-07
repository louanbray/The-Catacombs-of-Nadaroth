#ifndef STATISTICS_MANAGER_H
#define STATISTICS_MANAGER_H

enum StatisticID {
    STAT_ENEMIES_KILLED,              // OK
    STAT_CHEST_OPENED,                // OK
    STAT_DISTANCE_TRAVELED,           // OK
    STAT_TIME_PLAYED,                 // OK
    STAT_GAME_COMPLETIONS,            // OK
    STAT_GAME_STARTED,                // OK
    STAT_GAME_COMPLETION_AS_BALL,     // OK
    STAT_GAME_COMPLETION_AS_CAMO,     // OK
    STAT_GAME_COMPLETION_AS_BRAWLER,  // OK
    STAT_GAME_COMPLETION_AS_SHIELD,   // OK
    STAT_SPEED_RUNS,                  // OK
    STATISTIC_COUNT,
};

/**
 * Increase a statistic by the given amount.
 *
 * The id is ignored when out of range. Updated values are persisted automatically,
 * except that distance traveled is only saved every 10 points.
 */
void increment_statistic(enum StatisticID id, int amount);

/**
 * Get the current value of a statistic.
 *
 * Returns 0 when the id is out of range.
 */
int get_statistic(enum StatisticID id);

/**
 * Load statistics from the persistent statistics file.
 *
 * Missing files leave the in-memory statistics initialized to zero.
 */
void load_statistics();

/**
 * Save all statistics to the persistent statistics file.
 */
void save_statistics();

#endif