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

void set_chunk_type(chunk* ck, int type) {
    ck->type = type;
}

/// @brief Copy the content of the items dynarrray to put it into the hashmap
/// @param c chunk
void fill_chunk_hm_from_dyn(chunk* c) {
    dynarray* dyn = get_chunk_furniture_list(c);
    hm* hashmap = get_chunk_furniture_coords(c);
    for (int i = 0; i < len_dyn(dyn); i++) {
        void* e = get_dyn(dyn, i);
        set_hm(hashmap, get_item_x(e), get_item_y(e), e);
    }
}

/// @brief Add all elements of the chunk depending of the type
/// @param c chunk
/// @param type type
void fill_furniture(chunk* c, int type) {
    parse_chunk(get_chunk_furniture_list(c), type);
    fill_chunk_hm_from_dyn(c);
}

void decorate(chunk* c, int x, int y) {
    int type = SPAWN;
    if (x != 0 || y != 0) {
        type = SPAWN;  // TODO: DARK MAGIC
    }
    fill_furniture(c, type);
}
