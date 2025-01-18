#include "entity.h"

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

    switch (dir) {
        case EAST:
            x += 2;
            break;
        case WEST:
            x -= 2;
            break;
        case NORTH:
            y++;
            break;
        case SOUTH:
            y--;
            break;
        default:
            break;
    }

    hm* h = c->hashmap;
    purge_hm(h, get_item_x(i), get_item_y(i));
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
    for (int i = 0; i < len; i++) {
        item* it = get_dyn(d, i);
        if (it != NULL) {
            int x = get_item_x(it);
            int y = get_item_y(it);
            switch (dir) {
                case EAST:
                    x += 2;
                    break;
                case WEST:
                    x -= 2;
                    break;
                case NORTH:
                    y++;
                    break;
                case SOUTH:
                    y--;
                    break;
                default:
                    break;
            }
            if (!is_in_box(x, y)) {
                return false;
            }
            hm* h = get_chunk_furniture_coords(e->c);
            item* it2 = get_hm(h, x, y);
            if (it2 != NULL && get_entity_link(it2) != get_entity_link(it2)) {
                return false;
            }
        }
    }
    return true;
}

void move_entity(entity* e, enum Direction dir) {
    dynarray* d = e->parts;
    int len = len_dyn(d);
    if (!can_entity_move(e, dir)) {
        return;
    }
    for (int i = 0; i < len; i++) {
        item* it = get_dyn(d, i);
        if (it != NULL) {
            move_part(e->c, it, dir);
        }
    }
}