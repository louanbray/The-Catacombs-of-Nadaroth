#include "player.h"

#include <time.h>

#include "achievements.h"
#include "assets_manager.h"
#include "handler.h"
#include "logger.h"
#include "map.h"
#include "projectile.h"
#include "statistics.h"

static int CAN_DIE = true;
static int last_hotbar_index = 0;

#define DEFAULT_DAMAGE 1
#define DEFAULT_ARROW_SPEED 6
#define DEFAULT_INFINITY false

typedef struct PlayerClassModifiers {
    float additional_damage;
    int additional_arrow_speed;
    bool accuracy_mode;
    int aggro_range;
    int range;
    int health;
    int max_health;
    int player_design;
} PlayerClassModifiers;

static const PlayerClassModifiers BALL_MODIFIER = {0.0f, 0, false, -1, -1, 2, 5, PLAYER_DESIGN_BALL};
static const PlayerClassModifiers CAMO_MODIFIER = {0.25f, 2, true, 15, -1, 1, 3, PLAYER_DESIGN_CAMO};
static const PlayerClassModifiers BRAWLER_MODIFIER = {0.5f, -2, false, -1, 10, 4, 8, PLAYER_DESIGN_BRAWLER};
static const PlayerClassModifiers SHIELD_MODIFIER = {-0.25f, -1, false, -1, -1, 5, 10, PLAYER_DESIGN_SHIELD};

static const PlayerClassModifiers CLASS_MODIFIERS[PLAYER_CLASS_COUNT] = {BALL_MODIFIER, CAMO_MODIFIER, BRAWLER_MODIFIER, SHIELD_MODIFIER};

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
    int start_health, start_max_health;
    float additional_damage;
    int additional_arrow_speed;
    bool accuracy_mode;
    int aggro_range;
    int time_survivor_in_chunk;
    PlayerClass class;
} player;

static int min(int x, int y) {
    return x < y ? x : y;
}

/// @brief Set player pos to chunk center
/// @param p
void center_player(player* p) {
    p->x = get_chunk_spawn_x(p->current_chunk);
    p->y = get_chunk_spawn_y(p->current_chunk);
    p->px = p->x;
    p->py = p->y;
}

void destroy_player(player* p) {
    if (p == NULL) return;
    if (p->name != NULL)
        free(p->name);
    free(p);
}

player* create_player(map* m) {
    player* p = malloc(sizeof(player));
    p->current_chunk = get_spawn(m);
    p->score = 0;
    p->deaths = 0;
    p->start_health = 2;
    p->start_max_health = 5;
    p->health = p->start_health;
    p->mental_health = MAX_MENTAL_HEALTH;
    p->max_health = p->start_max_health;
    p->damage = DEFAULT_DAMAGE;
    p->arrow_speed = DEFAULT_ARROW_SPEED;
    p->range = -1;
    p->infinity = DEFAULT_INFINITY;
    p->hotbar = NULL;
    p->design = PLAYER_DESIGN_BALL;
    p->name = NULL;
    p->map = m;
    p->phase = GAMEPHASE_INTRODUCTION;
    p->color = COLOR_YELLOW;
    p->additional_damage = 0.0f;
    p->additional_arrow_speed = 0;
    p->accuracy_mode = false;
    p->aggro_range = -1;
    p->time_survivor_in_chunk = -1;
    center_player(p);
    set_map_player(m, p);
    return p;
}

void player_death(player* p) {
    if (p->phase == GAMEPHASE_FIRST_ACT_END) return;
    reset_total_enemies();
    chunk* spawn_chunk = get_spawn(get_player_map(p));
    p->current_chunk = spawn_chunk;
    p->x = get_chunk_spawn_x(spawn_chunk);
    p->y = get_chunk_spawn_y(spawn_chunk);
    p->health = p->start_health;
    p->px = p->x;
    p->py = p->y;
    if (p->score >= ScorePerPhase[p->phase] || p->phase == GAMEPHASE_INTRODUCTION) {
        p->phase++;
    } else {
        modify_player_mental_health(p, -1);
    }
    p->score = 0;
    p->deaths++;
    add_total_enemies(p);
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

void set_player_x(player* p, int x) {
    if (p) p->x = x;
}

void set_player_y(player* p, int y) {
    if (p) p->y = y;
}

void set_player_px(player* p, int px) {
    if (p) p->px = px;
}

void set_player_py(player* p, int py) {
    if (p) p->py = py;
}

chunk* get_player_chunk(player* p) {
    return p->current_chunk;
}

void set_player_chunk(player* p, chunk* ck) {
    p->current_chunk = ck;
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

int get_player_base_damage(player* p) {
    return p->damage;
}

int get_player_damage(player* p) {
    return p->damage + (int)(p->damage * p->additional_damage);
}

int get_player_range(player* p) {
    return p->range;
}

bool has_infinity(player* p) {
    return p->infinity && (!p->accuracy_mode);
}

int get_player_score(player* p) {
    return p->score;
}

int get_player_raw_arrow_speed(player* p) {
    return p->arrow_speed;
}

int get_player_arrow_speed(player* p) {
    return p->arrow_speed - p->additional_arrow_speed > 0 ? p->arrow_speed - p->additional_arrow_speed : 1;
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

PlayerClass get_player_class(player* p) {
    return p->class;
}

void increment_player_phase(player* p) {
    p->phase++;
}

void add_player_deaths(player* p) {
    p->deaths++;
}

void set_player_deaths(player* p, int deaths) {
    if (p) p->deaths = deaths;
}

void set_player_mental_health(player* p, int mental_health) {
    p->mental_health = mental_health;
}

void modify_player_mental_health(player* p, int mental_health) {
    p->mental_health += mental_health;
    if (p->mental_health < 0) p->mental_health = 0;
    if (p->mental_health > MAX_MENTAL_HEALTH) p->mental_health = MAX_MENTAL_HEALTH;
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

void set_player_max_health(player* p, int health) {
    p->max_health = health;
}

void set_player_damage(player* p, int damage) {
    p->damage = damage;
}

void set_player_color(player* p, Color color) {
    p->color = color;
}

void set_player_design(player* p, int design) {
    p->design = design;
}

void set_player_health_raw(player* p, int health) {
    p->health = health;
}

void set_player_class(player* p, PlayerClass class) {
    if (class > PLAYER_CLASS_COUNT || class < 0) class = PLAYER_CLASS_BALL;
    p->additional_damage = CLASS_MODIFIERS[class].additional_damage;
    p->additional_arrow_speed = CLASS_MODIFIERS[class].additional_arrow_speed;
    p->accuracy_mode = CLASS_MODIFIERS[class].accuracy_mode;
    p->aggro_range = CLASS_MODIFIERS[class].aggro_range;
    p->range = CLASS_MODIFIERS[class].range;
    p->start_health = CLASS_MODIFIERS[class].health;
    p->start_max_health = CLASS_MODIFIERS[class].max_health;
    p->design = CLASS_MODIFIERS[class].player_design;

    p->health = p->start_health;
    p->max_health = p->start_max_health;
    p->class = class;
}

void link_hotbar(player* p, hotbar* h) {
    p->hotbar = h;
}

PlayerMovementResult move_player(player* p, Direction dir) {
    const int dx[] = {0, 2, 0, -2, 0};
    const int dy[] = {0, 0, 1, 0, -1};

    p->px = p->x;
    p->py = p->y;

    int new_x = p->x + dx[dir];
    int new_y = p->y + dy[dir];

    if (!is_in_box(new_x, new_y)) return MOV_CANT_MOVE;

    PlayerMovementResult n = handle(p, new_x, new_y);

    if (n == MOV_CAN_MOVE || n == MOV_PICKED_UP || n == MOV_PICKED_UP_ENTITY) {
        p->x = new_x;
        p->y = new_y;
        increment_statistic(STAT_DISTANCE_TRAVELED, 1);
    }

    return n;
}

void move_player_chunk(player* p, Direction dir) {
    p->current_chunk = get_chunk_from(p->map, p->current_chunk, dir);
    bool new_chunk = is_new_chunk();
    if (new_chunk) add_total_enemies(p);
    if (new_chunk && p->health == 1) {
        p->time_survivor_in_chunk = 10;
        LOG_INFO("Survivor achievement countdown started.");
    } else {
        set_achievement_progress(ACH_SURVIVOR, 0);
        p->time_survivor_in_chunk = -1;
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
        p->time_survivor_in_chunk = -1;
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
        p->time_survivor_in_chunk = -1;
    }
    p->health = min(p->health + heal, p->max_health);
}

void destroy_player_cchunk(player* p) {
    map* m = p->map;
    chunk* c = p->current_chunk;
    destroy_chunk(m, get_chunk(m, get_chunk_x(c), get_chunk_y(c) + 1));
}

int distance_to_player_sq(player* p, int x, int y) {
    int dx = (p->x + RECENTER_X - x) / 2;
    int dy = -p->y + RECENTER_Y - y;
    return dx * dx + dy * dy;
}

bool is_player_aggroed(player* p, int x, int y) {
    if (p->aggro_range == -1) return true;
    return distance_to_player_sq(p, x, y) <= p->aggro_range * p->aggro_range;
}

void set_player_can_die(bool can_die) {
    CAN_DIE = can_die;
}

bool can_player_die() {
    return CAN_DIE;
}

void survivor_countdown(player* p, int seconds) {
    if (p->time_survivor_in_chunk == -1) return;
    p->time_survivor_in_chunk -= seconds;
    if (p->time_survivor_in_chunk <= 0) {
        add_achievement_progress(ACH_SURVIVOR, 1);
        p->time_survivor_in_chunk = -1;
    }
}

void bow_check_flag() {
    last_hotbar_index = -1;
}

void player_update_weapon(player* p) {
    int index = get_selected_slot(p->hotbar);
    if (index != last_hotbar_index) {
        last_hotbar_index = index;
        item* it = get_selected_item(p->hotbar);
        p->damage = DEFAULT_DAMAGE;
        p->infinity = DEFAULT_INFINITY;
        p->arrow_speed = DEFAULT_ARROW_SPEED;
        if (it != NULL) {
            UsableItem type = get_item_usable_type(it);
            if (type < USABLE_ITEM_BOWS_END && type != USABLE_ITEM_NOT_USABLE) {
                p->damage = get_usable_item_file(type)->specs.specs[1];
                p->infinity = get_usable_item_file(type)->specs.specs[2];
                p->arrow_speed = get_usable_item_file(type)->specs.specs[3];
            }
        }
    }
}