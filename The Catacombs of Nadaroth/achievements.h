#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

#include <stdbool.h>

enum AchievementID {
    ACH_FIRST_BLOOD,       // OK
    ACH_MONSTER_HUNTER,    // OK
    ACH_TREASURE_SEEKER,   // OK
    ACH_UNSTOPPABLE,       // OK
    ACH_MASTER_EXPLORER,   // OK
    ACH_PUZZLE_SOLVER,     // NOK
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

int get_achievement_points(enum AchievementID id);
int get_achievement_progress(enum AchievementID id);
int get_achievement_max_progress(enum AchievementID id);
int get_total_points();
const char* get_achievement_name(enum AchievementID id);
const char* get_achievement_description(enum AchievementID id);
void set_achievement_progress(enum AchievementID id, int progress);
void add_achievement_progress(enum AchievementID id, int progress);
bool is_achievement_unlocked(enum AchievementID id);
void load_achievements();
void save_achievements();
// void display_achievements(Render_Buffer* r);

#endif