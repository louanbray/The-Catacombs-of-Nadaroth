#include "player.h"

#include <time.h>

#include "achievements.h"
#include "handler.h"
#include "logger.h"
#include "map.h"
#include "statistics.h"

static int START_HEALTH = 2;
static int START_MAX_HEALTH = 5;
static int ADDITIONAL_HEALTH = 0;
static int ADDITIONAL_MAX_HEALTH = 0;
static float ADDITIONAL_DAMAGE = 0.0;
static int ADDITIONAL_ARROW_SPEED = 0;
static int RANGE = -1;
static bool ACCURACY_MODE = false;
static int AGGRO_RANGE = -1;
static int CAN_DIE = true;

static int TIME_SURVIVOR_IN_CHUNK = -1;

typedef struct player {
    map* map;
    int x, y, px, py;
    chunk* current_chunk;
    hotbar* hotbar;
    int health, max_health, mental_health;
    int damage, arrow_speed;
    int range, infinity;
    int design;
    Color color;
    int score, deaths;
    GamePhase phase;
    char* name;
} player;

/// @brief Set player pos to chunk center
/// @param p
void center_player(player* p) {
    p->x = get_chunk_spawn_x(p->current_chunk);
    p->y = get_chunk_spawn_y(p->current_chunk);
    p->px = p->x;
    p->py = p->y;
}

player* create_player(map* m) {
    player* p = malloc(sizeof(player));
    p->current_chunk = get_spawn(m);
    p->score = 0;
    p->deaths = 0;
    p->health = START_HEALTH;
    p->mental_health = 4;
    p->max_health = START_MAX_HEALTH;
    p->damage = 1;
    p->arrow_speed = 6;
    p->range = RANGE;
    p->infinity = false;
    p->hotbar = NULL;
    p->design = PLAYER_DESIGN_BALL;
    p->name = NULL;
    p->map = m;
    p->phase = INTRODUCTION;
    p->color = COLOR_GREEN;
    center_player(p);
    set_map_player(m, p);
    return p;
}

void player_death(player* p) {
    if (p->phase == FIRST_ACT_END) return;
    chunk* spawn_chunk = get_spawn(get_player_map(p));
    p->current_chunk = spawn_chunk;
    p->x = get_chunk_spawn_x(spawn_chunk);
    p->y = get_chunk_spawn_y(spawn_chunk);
    p->health = START_HEALTH;
    p->px = p->x;
    p->py = p->y;
    if (p->score >= ScorePerPhase[p->phase] || p->phase == INTRODUCTION) {
        p->phase++;
    } else {
        modify_player_mental_health(p, -1);
    }
    p->score = 0;
    p->deaths++;
}

int get_player_x(player* p) {
    return p->x;
}

int get_player_y(player* p) {
    return p->y;
}

int get_player_px(player* p) {
    return p->px;
}

int get_player_py(player* p) {
    return p->py;
}

chunk* get_player_chunk(player* p) {
    return p->current_chunk;
}

hotbar* get_player_hotbar(player* p) {
    return p->hotbar;
}

int get_player_design(player* p) {
    return p->design;
}

char* get_player_name(player* p) {
    return p->name;
}

map* get_player_map(player* p) {
    return p->map;
}

int get_player_health(player* p) {
    return p->health;
}

int get_player_max_health(player* p) {
    return p->max_health;
}

int get_player_damage(player* p) {
    return p->damage + (int)(p->damage * ADDITIONAL_DAMAGE);
}

int get_player_range(player* p) {
    return p->range;
}

bool has_infinity(player* p) {
    return p->infinity && (!ACCURACY_MODE);
}

int get_player_score(player* p) {
    return p->score;
}

int get_player_arrow_speed(player* p) {
    return p->arrow_speed - ADDITIONAL_ARROW_SPEED;
}

int get_player_deaths(player* p) {
    return p->deaths;
}

int get_player_mental_health(player* p) {
    return p->mental_health;
}

GamePhase get_player_phase(player* p) {
    return p->phase;
}

Color get_player_color(player* p) {
    return p->color;
}

void set_player_phase(player* p, GamePhase phase) {
    p->phase = phase;
}

void increment_player_phase(player* p) {
    p->phase++;
}

void add_player_deaths(player* p) {
    p->deaths++;
}

void set_player_mental_health(player* p, int mental_health) {
    p->mental_health = mental_health;
}

void modify_player_mental_health(player* p, int mental_health) {
    p->mental_health += mental_health;
    if (p->mental_health < 0) p->mental_health = 0;
    if (p->mental_health > 4) p->mental_health = 4;
}

void set_player_arrow_speed(player* p, int speed) {
    p->arrow_speed = speed;
}

void set_player_range(player* p, int range) {
    p->range = range;
}

void set_player_score(player* p, int score) {
    p->score = score;
}

void set_player_infinity(player* p, bool infinite) {
    p->infinity = infinite;
}

void add_player_score(player* p, int score) {
    p->score += score;
}

void set_player_max_health(player* p, unsigned int health) {
    p->max_health = health;
}

void set_player_damage(player* p, unsigned int damage) {
    p->damage = damage;
}

void set_player_color(player* p, Color color) {
    p->color = color;
}

void set_player_design(player* p, int design) {
    p->design = design;
}

void link_hotbar(player* p, hotbar* h) {
    p->hotbar = h;
}

int move_player(player* p, Direction dir) {
    const int dx[] = {0, 2, 0, -2, 0};
    const int dy[] = {0, 0, 1, 0, -1};

    p->px = p->x;
    p->py = p->y;

    int new_x = p->x + dx[dir];
    int new_y = p->y + dy[dir];

    if (!is_in_box(new_x, new_y))
        return CANT_MOVE;

    int n = handle(p, new_x, new_y);

    if (n == CAN_MOVE || n == PICKED_UP || n == PICKED_UP_ENTITY) {
        p->x = new_x;
        p->y = new_y;
        increment_statistic(STAT_DISTANCE_TRAVELED, 1);
    }

    return n;
}

void move_player_chunk(player* p, Direction dir) {
    p->current_chunk = get_chunk_from(p->map, p->current_chunk, dir);
    LOG_INFO("Is new chunk generated: %s", is_new_chunk() ? "true" : "false");
    if (is_new_chunk() && p->health == 1) {
        TIME_SURVIVOR_IN_CHUNK = 10;
        LOG_INFO("Survivor achievement countdown started.");
    } else {
        set_achievement_progress(ACH_SURVIVOR, 0);
        TIME_SURVIVOR_IN_CHUNK = -1;
    }
    center_player(p);
    reset_new_chunk_flag();
}

bool damage_player(player* p, int damage) {
    if (!CAN_DIE) return false;
    if (damage > 0) {
        set_achievement_progress(ACH_FIRST_BLOOD, 1);
        set_achievement_progress(ACH_UNSTOPPABLE, 0);
        set_achievement_progress(ACH_SURVIVOR, 0);
        TIME_SURVIVOR_IN_CHUNK = -1;
    }
    if (p->health - damage <= 0) {
        set_achievement_progress(ACH_MASTER_EXPLORER, 0);
        p->health = 0;
        return true;
    }
    p->health += -damage;
    return false;
}

void heal_player(player* p, int heal) {
    if (heal > 0) {
        set_achievement_progress(ACH_SURVIVOR, 0);
        TIME_SURVIVOR_IN_CHUNK = -1;
    }
    if (p->health + heal > p->max_health) {
        p->health = p->max_health;
        return;
    }
    p->health += heal;
}

void set_player_class(player* p, int class) {
    if (class == 0)
        p->design = PLAYER_DESIGN_BALL;
    else if (class == 1) {
        p->design = PLAYER_DESIGN_CAMO;
        ADDITIONAL_DAMAGE = 0.25;
        ADDITIONAL_HEALTH = -1;
        ADDITIONAL_MAX_HEALTH = -2;
        ADDITIONAL_ARROW_SPEED = 2;
        ACCURACY_MODE = true;
        AGGRO_RANGE = 15;
    } else if (class == 2) {
        p->design = PLAYER_DESIGN_BRAWLER;
        ADDITIONAL_DAMAGE = 0.5;
        ADDITIONAL_HEALTH = 2;
        ADDITIONAL_MAX_HEALTH = 3;
        ADDITIONAL_ARROW_SPEED = -2;
        RANGE = 10;
    } else if (class == 3) {
        p->design = PLAYER_DESIGN_SHIELD;
        ADDITIONAL_DAMAGE = -0.25;
        ADDITIONAL_HEALTH = 3;
        ADDITIONAL_MAX_HEALTH = 5;
        ADDITIONAL_ARROW_SPEED = -1;
    }
    START_HEALTH += ADDITIONAL_HEALTH;
    START_MAX_HEALTH += ADDITIONAL_MAX_HEALTH;
    p->health = START_HEALTH;
    p->max_health = START_MAX_HEALTH;
    p->range = RANGE;
}

int get_player_class(player* p) {
    if (p->design == PLAYER_DESIGN_BALL)
        return 0;
    else if (p->design == PLAYER_DESIGN_CAMO)
        return 1;
    else if (p->design == PLAYER_DESIGN_BRAWLER)
        return 2;
    else if (p->design == PLAYER_DESIGN_SHIELD)
        return 3;
    return -1;
}

void destroy_player_cchunk(player* p) {
    map* m = p->map;
    chunk* c = p->current_chunk;
    destroy_chunk(m, get_chunk(m, get_chunk_x(c), get_chunk_y(c) + 1));
}

int distance_to_player_sq(player* p, int x, int y) {
    int dx = (get_player_x(p) + 65 - x) / 2;
    int dy = -get_player_y(p) + 19 - y;
    return dx * dx + dy * dy;
}

bool is_player_aggroed(player* p, int x, int y) {
    if (AGGRO_RANGE == -1) return true;
    return distance_to_player_sq(p, x, y) <= AGGRO_RANGE * AGGRO_RANGE;
}

void set_player_can_die(bool can_die) {
    CAN_DIE = can_die;
}

bool can_player_die() {
    return CAN_DIE;
}

void survivor_countdown(int seconds) {
    if (TIME_SURVIVOR_IN_CHUNK == -1) return;
    TIME_SURVIVOR_IN_CHUNK -= seconds;
    if (TIME_SURVIVOR_IN_CHUNK <= 0) {
        add_achievement_progress(ACH_SURVIVOR, 1);
        TIME_SURVIVOR_IN_CHUNK = -1;
    }
}