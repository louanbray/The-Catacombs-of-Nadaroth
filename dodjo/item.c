#include "item.h"

typedef struct item {
    int x, y, type;
    bool hidden, used;
    int display, index;
    void* spec;
} item;

/// @brief Create item using given parameters
/// @param x pos x
/// @param y pos y
/// @param type Type
/// @return item
item* create_item(int x, int y, int type, int display, int index) {
    item* i = malloc(sizeof(item));
    i->x = x;
    i->y = y;
    i->type = type;
    i->display = display;
    i->index = index;
    return i;
}

/// @brief Complete the item specs depending on its type
/// @param i item
/// @param type type
void specialize(item* i, int type) {
    switch (type) {  //? Modify to add different types of items
        default:
            i->hidden = false;
            i->used = false;
            i->spec = NULL;
            break;
    }
}

item* generate_item(int x, int y, int type, int display, int index) {
    item* i = create_item(x, y, type, display, index);
    specialize(i, type);
    return i;
}

int get_item_x(item* i) {
    return i->x;
}

int get_item_y(item* i) {
    return i->y;
}

int get_item_type(item* i) {
    return i->type;
}

int get_item_display(item* i) {
    return i->display;
}

int get_item_index(item* i) {
    return i->index;
}

void set_item_index(item* i, int index) {
    i->index = index;
}

void* get_item_spec(item* i) {
    return i->spec;
}

bool is_item_hidden(item* i) {
    return i->hidden;
}

bool is_item_used(item* i) {
    return i->used;
}

void set_item_hidden(item* i, bool hidden) {
    i->hidden = hidden;
}

void set_item_used(item* i, bool used) {
    i->used = used;
}

void set_item_display(item* i, int display) {
    i->display = display;
}

void set_item_spec(item* i, void* spec) {
    i->spec = spec;
}

void free_item(item* i) {
    if (i->spec != NULL) {
        free(i->spec);
    }
    free(i);
}