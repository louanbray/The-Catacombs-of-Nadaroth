#include "player.h"

#include "handler.h"
#include "map.h"

typedef struct player {
    map* map;
    int x, y, px, py;
    chunk* current_chunk;
    hotbar* hotbar;
    int health;
    int design;
    char* name;
} player;

void center_player(player* p) {
    p->x = 0;
    p->y = 0;
    p->px = 0;
    p->py = 0;
}

player* create_player(map* m) {
    player* p = malloc(sizeof(player));
    center_player(p);
    p->current_chunk = get_spawn(m);
    p->health = 1;
    p->hotbar = NULL;
    p->design = 3486;
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

int get_player_design(player* p) {
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

int move_player(player* p, int dir) {
    int s = dir < 3 ? 1 : -1;
    p->px = p->x;
    p->py = p->y;
    int new_x = p->x + dir % 2 * s;
    int new_y = p->y + s * (dir - 1) % 2;
    if (!is_in_box(new_x, new_y))
        return 2;
    if (handle(p, new_x, new_y)) {
        p->x = new_x;
        p->y = new_y;
        return 0;
    }
    return 1;
}

void move_player_chunk(player* p, int dir) {
    center_player(p);
    p->current_chunk = get_chunk_from(p->map, p->current_chunk, dir);
}

void damage_player(player* p, int damage) {
    if (p->health - damage < 0) {
        p->health = 0;
        return;
    }
    p->health += -damage;
}