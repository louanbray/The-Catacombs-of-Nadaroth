#include "entity.h"

#include <omp.h>
typedef struct entity {
    dynarray* parts;
    item* brain;
    chunk* c;
} entity;

entity* create_entity(item* brain, chunk* c) {
    entity* e = malloc(sizeof(entity));
    e->parts = create_dyn();
    e->brain = brain;
    e->c = c;
    return e;
}

item* get_entity_brain(entity* e) {
    return e->brain;
}

dynarray* get_entity_parts(entity* e) {
    return e->parts;
}

void add_entity_part(entity* e, item* i) {
    append(e->parts, i);
}

void for_each_entity_part(entity* e, void (*f)(item*)) {
    dynarray* d = e->parts;
    int len = len_dyn(d);
    for (int i = 0; i < len; i++) {
        f(get_dyn(d, i));
    }
}

void destroy_entity(entity* e) {
    free_dyn(e->parts);
    free_item(e->brain);
    free(e);
}

void remove_entity_from_chunk(entity* e) {
    chunk* c = e->c;
    dynarray* d = e->parts;
    int len = len_dyn(d);
    for (int i = 0; i < len; i++) {
        item* it = get_dyn(d, i);
        remove_item(c, it);
        set_dyn(d, i, NULL);
        free_item(it);
    }
}

void destroy_entity_from_chunk(entity* e) {
    remove_entity_from_chunk(e);
    destroy_entity(e);
}

/// @brief Move entity part in the given direction
/// @param c entity chunk
/// @param i entity part
/// @param dir direction
void move_part(chunk* c, item* i, enum Direction dir) {
    int x = get_item_x(i);
    int y = get_item_y(i);

    const int dx[] = {0, 2, 0, -2, 0};
    const int dy[] = {0, 0, 1, 0, -1};

    hm* h = c->hashmap;
    purge_hm(h, x, y);

    x += dx[dir];
    y += dy[dir];

    set_item_x(i, x);
    set_item_y(i, y);
    set_hm(h, x, y, i);
}

// TODO: (THIS IS NOT OPTIMIZED) | (HIT BOX ?) ->
/// @brief Check if the entity can move in the given direction
/// @param e entity
/// @param dir direction
bool can_entity_move(entity* e, enum Direction dir) {
    dynarray* d = e->parts;
    int len = len_dyn(d);

    const int dx[] = {0, 2, 0, -2, 0};
    const int dy[] = {0, 0, 1, 0, -1};

    hm* h = get_chunk_furniture_coords(e->c);
    bool can_move = true;

#pragma omp parallel for shared(can_move)
    for (int i = 0; i < len; i++) {
        if (!can_move) continue;

        item* it = get_dyn(d, i);
        if (it != NULL) {
            int x = get_item_x(it) + dx[dir];
            int y = get_item_y(it) + dy[dir];

            if (!is_in_box(x, y)) {
#pragma omp atomic write
                can_move = false;
            }

            item* it2 = get_hm(h, x, y);
            if (it2 != NULL && get_entity_link(it2) != e) {
#pragma omp atomic write
                can_move = false;
            }
        }
    }

    return can_move;
}

void move_entity(entity* e, enum Direction dir) {
    if (!can_entity_move(e, dir)) {
        return;
    }

    dynarray* d = e->parts;
    int len = len_dyn(d);

    for (int i = 0; i < len; i++) {
        item* it = get_dyn(d, i);
        if (it != NULL) {
            move_part(e->c, it, dir);
        }
    }
}