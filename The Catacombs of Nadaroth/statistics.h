#ifndef STATISTICS_H
#define STATISTICS_H

enum StatisticID {
    STAT_ENEMIES_KILLED,
    STAT_CHEST_OPENED,
    STAT_DISTANCE_TRAVELED,
    STAT_TIME_PLAYED,
    STATISTIC_COUNT,
};

void increment_statistic(enum StatisticID id, int amount);
int get_statistic(enum StatisticID id);
void load_statistics();
void save_statistics();

#endif