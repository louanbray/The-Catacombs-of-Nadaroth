#ifndef STATISTICS_H
#define STATISTICS_H

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

void increment_statistic(enum StatisticID id, int amount);
int get_statistic(enum StatisticID id);
void load_statistics();
void save_statistics();

#endif