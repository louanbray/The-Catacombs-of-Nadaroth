#include "player.h"

#include "handler.h"
#include "map.h"

#define START_HEALTH 2

typedef struct player {
    map* map;
    int x, y, px, py;
    chunk* current_chunk;
    hotbar* hotbar;
    int health, max_health, mental_health;
    int damage, arrow_speed;
    bool infinite_range;
    int design;
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
    p->max_health = 5;
    p->damage = 1;
    p->arrow_speed = 6;
    p->infinite_range = false;
    p->hotbar = NULL;
    p->design = 9210;
    p->name = NULL;
    p->map = m;
    p->phase = INTRODUCTION;
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
    return p->damage;
}

bool has_infinite_range(player* p) {
    return p->infinite_range;
}

int get_player_score(player* p) {
    return p->score;
}

int get_player_arrow_speed(player* p) {
    return p->arrow_speed;
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

void set_player_infinite_range(player* p, bool infinite) {
    p->infinite_range = infinite;
}

void set_player_score(player* p, int score) {
    p->score = score;
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
        return 2;

    int n = handle(p, new_x, new_y);

    if (n == 0 || n == 3 || n == 4) {
        p->x = new_x;
        p->y = new_y;
    }

    return n;
}

void move_player_chunk(player* p, Direction dir) {
    p->current_chunk = get_chunk_from(p->map, p->current_chunk, dir);
    center_player(p);
}

bool damage_player(player* p, int damage) {
    if (p->health - damage <= 0) {
        p->health = 0;
        return true;
    }
    p->health += -damage;
    return false;
}

void heal_player(player* p, int heal) {
    if (p->health + heal > p->max_health) {
        p->health = p->max_health;
        return;
    }
    p->health += heal;
}

void destroy_player_cchunk(player* p) {
    map* m = p->map;
    chunk* c = p->current_chunk;
    destroy_chunk(m, get_chunk(m, get_chunk_x(c), get_chunk_y(c) + 1));
}