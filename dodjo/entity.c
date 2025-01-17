#include "entity.h"

#include <wchar.h>
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