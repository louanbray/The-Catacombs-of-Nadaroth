#include "statistics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STAT_FILE "assets/data/player_statistics.dodjo"

static int* statistics = NULL;

void increment_statistic(enum StatisticID id, int amount) {
    if (id < 0 || id >= STATISTIC_COUNT) return;
    statistics[id] += amount;
    if (amount != 0 && (id != STAT_DISTANCE_TRAVELED || statistics[id] % 10 == 0)) save_statistics();
}

int get_statistic(enum StatisticID id) {
    if (id < 0 || id >= STATISTIC_COUNT) return 0;
    return statistics[id];
}

void load_statistics() {
    statistics = calloc(sizeof(int), STATISTIC_COUNT);
    FILE* file = fopen(STAT_FILE, "r");
    if (file == NULL) return;
    for (int i = 0; i < STATISTIC_COUNT; i++) {
        fscanf(file, "%d\n", &statistics[i]);
    }
    fclose(file);
}

void save_statistics() {
    FILE* file = fopen(STAT_FILE, "w");
    if (file == NULL) return;
    for (int i = 0; i < STATISTIC_COUNT; i++) {
        fprintf(file, "%d\n", statistics[i]);
    }
    fclose(file);
}