#include "generation.h"

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

/// @brief Copy the content of the items dynarrray to put it into the hashmap
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
    int type = SPAWN;
    int spawn_x = 1;  //! TO CENTER THE PLAYER
    int spawn_y = 0;
    if (x != 0 || y != 0) {
        int t = rand() % 2;  //? MODIFY TO ADD A LEVEL (% Number of types)
        switch (t) {
            // case ?:
            //     spawn_x = ?;
            //     spawn_y = ?;
            //     type = DUMMY;
            //     break;
            case 1:
                type = SPAWN;
                break;
            default:
                type = DEFAULT2;
                break;
        }
    }
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
