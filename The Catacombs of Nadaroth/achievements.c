#include "achievements.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "logger.h"

#define PLAYER_FILE "assets/data/player_achievements.dodjo"
#define DATA_FILE "assets/data/achievements.dodjo"

typedef struct achievement {
    enum AchievementID id;
    const char* name;
    const char* description;
    int points;
    int max_progress;
    int progress;
} achievement;

achievement** achievements = NULL;

int get_achievement_points(enum AchievementID id) {
    int points = 0;
    if (is_achievement_unlocked(id)) points += achievements[id]->points;
    return points;
}

int get_total_points() {
    int points = 0;
    for (int i = 0; i < ACHIEVEMENT_COUNT; i++)
        if (is_achievement_unlocked((enum AchievementID)i)) points += achievements[i]->points;
    return points;
}

const char* get_achievement_name(enum AchievementID id) {
    if (id < 0 || id >= ACHIEVEMENT_COUNT) return NULL;
    return achievements[id]->name;
}

const char* get_achievement_description(enum AchievementID id) {
    if (id < 0 || id >= ACHIEVEMENT_COUNT) return NULL;
    return achievements[id]->description;
}

int get_achievement_progress(enum AchievementID id) {
    if (id < 0 || id >= ACHIEVEMENT_COUNT) return 0;
    return achievements[id]->progress;
}

int get_achievement_max_progress(enum AchievementID id) {
    if (id < 0 || id >= ACHIEVEMENT_COUNT) return 0;
    return achievements[id]->max_progress;
}

void set_achievement_progress(enum AchievementID id, int progress) {
    if (id < 0 || id >= ACHIEVEMENT_COUNT) return;
    if (achievements[id]->progress == achievements[id]->max_progress)
        return;
    if (progress > achievements[id]->max_progress)
        progress = achievements[id]->max_progress;
    if (progress != achievements[id]->progress) {
        achievements[id]->progress = progress;
        LOG_INFO("Achievement %s progress set to %d/%d", achievements[id]->name, progress, achievements[id]->max_progress);
        save_achievements();
    }
}

void add_achievement_progress(enum AchievementID id, int progress) {
    if (id < 0 || id >= ACHIEVEMENT_COUNT) return;
    if (achievements[id]->progress == achievements[id]->max_progress)
        return;
    int new_progress = achievements[id]->progress + progress;
    if (new_progress > achievements[id]->max_progress)
        new_progress = achievements[id]->max_progress;
    if (new_progress < 0)
        new_progress = 0;
    if (new_progress != achievements[id]->progress) {
        achievements[id]->progress = new_progress;
        LOG_INFO("Achievement %s progress increased to %d/%d", achievements[id]->name, new_progress, achievements[id]->max_progress);
        save_achievements();
    }
}

bool is_achievement_unlocked(enum AchievementID id) {
    if (id < 0 || id >= ACHIEVEMENT_COUNT) return false;
    return achievements[id]->progress == achievements[id]->max_progress;
}

void load_achievements() {
    achievements = calloc(sizeof(achievement*), ACHIEVEMENT_COUNT);
    FILE* player_file = fopen(PLAYER_FILE, "r");
    FILE* data_file = fopen(DATA_FILE, "r");

    if (data_file == NULL) return;
    while (!feof(data_file)) {
        int id;
        char name[256];
        char description[512];
        int points;
        int max_progress;
        fscanf(data_file, "%d;%255[^;];%511[^;];%d;%d\n", &id, name, description, &points, &max_progress);
        if (id >= 0 && id < ACHIEVEMENT_COUNT) {
            achievement* ach = malloc(sizeof(achievement));
            ach->id = (enum AchievementID)id;
            ach->name = strdup(name);
            ach->description = strdup(description);
            ach->points = points;
            ach->max_progress = max_progress;
            ach->progress = 0;
            achievements[id] = ach;
        }
    }

    int id = 0;
    while (player_file && !feof(player_file)) {
        int status = 0;
        fscanf(player_file, "%d\n", &status);
        if (id >= 0 && id < ACHIEVEMENT_COUNT) {
            achievements[id]->progress = status;
        }
        id++;
    }

    fclose(data_file);
    if (player_file) fclose(player_file);

    // Reset run based achievements (if uncompleted)
    set_achievement_progress(ACH_UNSTOPPABLE, 0);
    set_achievement_progress(ACH_MASTER_EXPLORER, 0);
    set_achievement_progress(ACH_SURVIVOR, 0);
    set_achievement_progress(ACH_CRAFTSMAN, 0);
    set_achievement_progress(ACH_GOURMET, 0);
}
void save_achievements() {
    FILE* file = fopen(PLAYER_FILE, "w");
    for (int i = 0; i < ACHIEVEMENT_COUNT; i++) {
        fprintf(file, "%d\n", achievements[i]->progress);
    }
    fclose(file);
}