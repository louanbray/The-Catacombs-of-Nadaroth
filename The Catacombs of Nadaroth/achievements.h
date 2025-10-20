#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

#include <stdbool.h>

enum AchievementID {
    ACH_FIRST_BLOOD,
    ACH_MONSTER_HUNTER,
    ACH_TREASURE_SEEKER,
    ACH_UNSTOPPABLE,
    ACH_MASTER_EXPLORER,
    ACH_PUZZLE_SOLVER,
    ACH_SPEED_RUNNER,
    ACH_SURVIVOR,
    ACH_LOOT_COLLECTOR,
    ACH_SECRET_FINDER,
    ACH_BOSS_SLAYER,
    ACH_PERFECTIONIST,
    ACH_CRAFTSMAN,
    ACH_ALCHEMIST,
    ACH_HERO_OF_NADAROTH,
    ACHIEVEMENT_COUNT,
};

int get_achievement_points(enum AchievementID id);
int get_total_points();
const char* get_achievement_name(enum AchievementID id);
const char* get_achievement_description(enum AchievementID id);
void set_achievement_progress(enum AchievementID id, int progress);
bool is_achievement_unlocked(enum AchievementID id);
void load_achievements();
void save_achievements();
// void display_achievements(Render_Buffer* r);

#endif