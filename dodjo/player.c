#include "player.h"

#include "handler.h"
#include "map.h"

typedef struct player {
    map* map;
    int x, y, px, py;
    chunk* current_chunk;
    hotbar* hotbar;
    int health, max_health;
    int design;
    char* name;
    /*
    TODO: add stats and achievements objects
    */
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
    p->health = 3;
    p->max_health = 3;
    p->hotbar = NULL;
    p->design = 3486;
    p->name = NULL;
    p->map = m;
    center_player(p);
    set_map_player(m, p);
    return p;
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

int get_player_health(player* p) {
    return p->health;
}

int get_player_max_health(player* p) {
    return p->max_health;
}

void set_player_max_health(player* p, unsigned int health) {
    p->max_health = health;
}

void link_hotbar(player* p, hotbar* h) {
    p->hotbar = h;
}

int move_player(player* p, int dir) {
    int s = dir < 3 ? 1 : -1;
    p->px = p->x;
    p->py = p->y;
    int new_x = p->x + (dir % 2 * s) * 2;
    int new_y = p->y + s * (dir - 1) % 2;
    if (!is_in_box(new_x, new_y))
        return 2;
    int n = handle(p, new_x, new_y);
    if (n == 0 || n == 3) {
        p->x = new_x;
        p->y = new_y;
    }
    return n;
}

void move_player_chunk(player* p, int dir) {
    p->current_chunk = get_chunk_from(p->map, p->current_chunk, dir);
    center_player(p);
}

void damage_player(player* p, int damage) {
    if (p->health - damage < 0) {
        p->health = 0;
        return;
    }
    p->health += -damage;
}

void heal_player(player* p, int heal) {
    if (p->health + heal > p->max_health) {
        p->health = p->max_health;
        return;
    }
    p->health += heal;
}