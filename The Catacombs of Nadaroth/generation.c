#include "generation.h"

#include <stdlib.h>

#include "game_status.h"
#include "item.h"

/// @brief Private chunk structure definition
typedef struct chunk {
    chunk_link link;
    int x, spawn_x;
    int y, spawn_y;
    ChunkType type;
    dynarray* elements;
    dynarray* enemies;
    hm* hashmap;
} chunk;

/// @brief Create an empty set of 5 chunk links
static chunk_link create_link() {
    return calloc(5, sizeof(chunk*));
}

dynarray* get_chunk_furniture_list(chunk* ck) {
    return ck->elements;
}

hm* get_chunk_furniture_coords(chunk* ck) {
    return ck->hashmap;
}

int get_chunk_x(chunk* c) {
    return c->x;
}

int get_chunk_y(chunk* c) {
    return c->y;
}

int get_chunk_spawn_x(chunk* c) {
    return c->spawn_x;
}

int get_chunk_spawn_y(chunk* c) {
    return c->spawn_y;
}

void set_chunk_type(chunk* ck, ChunkType type) {
    ck->type = type;
}

dynarray* get_chunk_enemies(chunk* ck) {
    return ck->enemies;
}

ChunkType get_chunk_type(chunk* ck) {
    return ck->type;
}

chunk_link get_chunk_links(chunk* ck) {
    return ck->link;
}

chunk* get_chunk_link_at(chunk* ck, int dir) {
    return ck->link[dir];
}

void set_chunk_link(chunk* ck, int dir, chunk* target) {
    ck->link[dir] = target;
}

void chunk_append_element(chunk* ck, item* it) {
    append(ck->elements, it);
}

void chunk_register_item(chunk* ck, item* it) {
    set_hm(ck->hashmap, get_item_x(it), get_item_y(it), it);
}

void chunk_append_enemy(chunk* ck, item* it) {
    append(ck->enemies, it);
}

int chunk_element_count(chunk* ck) {
    return len_dyn(ck->elements);
}

item* chunk_get_element(chunk* ck, int i) {
    return (item*)get_dyn(ck->elements, i);
}

chunk* create_chunk(int x, int y) {
    chunk* ck = malloc(sizeof(chunk));
    ck->link = create_link();
    ck->x = x;
    ck->y = y;
    ck->type = SPAWN;
    ck->spawn_x = 1;
    ck->spawn_y = 0;
    ck->elements = create_dyn();
    ck->enemies = create_dyn();
    ck->hashmap = create_hashmap();
    return ck;
}

chunk* generate_chunk(int x, int y) {
    chunk* c = create_chunk(x, y);
    decorate(c, x, y);
    return c;
}

chunk* create_chunk_raw(int x, int y, int spawn_x, int spawn_y, ChunkType type) {
    chunk* ck = malloc(sizeof(chunk));
    ck->link = create_link();
    ck->x = x;
    ck->y = y;
    ck->spawn_x = spawn_x;
    ck->spawn_y = spawn_y;
    ck->type = type;
    ck->elements = create_dyn();
    ck->enemies = create_dyn();
    ck->hashmap = create_hashmap();
    return ck;
}

void reset_chunk_internals(chunk* ck, int spawn_x, int spawn_y, ChunkType type) {
    int count = len_dyn(ck->elements);
    for (int i = 0; i < count; i++) {
        item* it = (item*)get_dyn(ck->elements, i);
        if (it != NULL) free_item(it);
    }
    free_dyn_no_item(ck->elements);
    free_dyn_no_item(ck->enemies);
    free_hm(ck->hashmap);
    ck->elements = create_dyn();
    ck->enemies = create_dyn();
    ck->hashmap = create_hashmap();
    ck->spawn_x = spawn_x;
    ck->spawn_y = spawn_y;
    ck->type = type;
}

void destroy_chunk_full(chunk* ck) {
    free(ck->link);
    free_dyn(ck->elements);
    free_dyn_no_item(ck->enemies);
    free_hm(ck->hashmap);
    free(ck);
}

/// @brief Copy the content of the items dynarray to put it into the hashmap
/// @param c chunk
void fill_chunk_hm_from_dyn(chunk* c) {
    dynarray* dyn = c->elements;
    hm* hashmap = c->hashmap;
    int len = len_dyn(dyn);
    for (int i = 0; i < len; i++) {
        item* e = get_dyn(dyn, i);
        if (e != NULL) {
            set_hm(hashmap, get_item_x(e), get_item_y(e), e);
        }
    }
}

/// @brief Add all elements of the chunk depending of the type
/// @param c chunk
/// @param type type
void fill_furniture(chunk* c, ChunkType type) {
    parse_chunk(c, c->elements, type);
    fill_chunk_hm_from_dyn(c);
}

void decorate(chunk* c, int x, int y) {
    int type = is_debug_mode() ? DEBUG : SPAWN;
    int spawn_x = 1;  //! TO CENTER THE PLAYER
    int spawn_y = 0;
    if (x != 0 || y != 0) type = 2 + (rand() % (CHUNK_TYPE_COUNT - 2));  //? MODIFY TO ADD A LEVEL (% Number of types) +2 to skip SPAWN and DEBUG
    // switch (type) {  //! temporary offline, need to incorporate spawn pos to chunk file (TODO or not if you spawn next to the gates)
    //     // case ?:
    //     //     spawn_x = ?;
    //     //     spawn_y = ?;
    //     //     type = DUMMY;
    //     //     break;
    //     case 1:
    //         type = SPAWN;
    //         break;
    //     default:
    //         type = DEFAULT2;
    //         break;
    // }

    c->type = type;
    c->spawn_x = spawn_x;
    c->spawn_y = spawn_y;
    fill_furniture(c, type);
}

void remove_item(chunk* ck, item* i) {
    if (i == NULL) return;
    hm* h = ck->hashmap;
    dynarray* d = ck->elements;
    purge_hm(h, get_item_x(i), get_item_y(i));
    set_dyn(d, get_item_index(i), NULL);
}
