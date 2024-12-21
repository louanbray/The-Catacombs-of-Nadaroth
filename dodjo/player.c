#include "player.h"

#include "map.h"

typedef struct player {
    map* map;
    int x, y, px, py;
    chunk* current_chunk;
    hotbar* hotbar;
    int health;
    char design;
    char* name;
} player;

player* create_player(map* m) {
    player* p = malloc(sizeof(player));
    p->x = 0;
    p->y = 0;
    p->px = 0;
    p->py = 0;
    p->current_chunk = get_spawn(m);
    p->health = 1;
    p->hotbar = NULL;
    p->design = 'O';
    p->name = NULL;
    p->map = m;
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

char get_player_design(player* p) {
    return p->design;
}

char* get_player_name(player* p) {
    return p->name;
}

int get_player_health(player* p) {
    return p->health;
}

void link_hotbar(player* p, hotbar* h) {
    p->hotbar = h;
}

void move_player(player* p, int dir) {
    int s = dir < 3 ? 1 : -1;
    p->px = p->x;
    p->py = p->y;
    p->x += dir % 2 * s;
    p->y += s * (dir - 1) % 2;
}

void move_player_chunk(player* p, int dir) {
    p->current_chunk = get_chunk_from(p->map, p->current_chunk, dir);
}

void damage_player(player* p, int damage) {
    if (p->health - damage < 0) {
        p->health = 0;
        return;
    }
    p->health += -damage;
}