#include "item.h"

typedef struct item {
    int x, y, type;
    bool hidden, used;
    int display;
    void* spec;
} item;

item* create_item(int x, int y, int type, int display) {
    item* i = malloc(sizeof(item));
    i->x = x;
    i->y = y;
    i->type = type;
    i->display = display;
    return i;
}

void specialize(item* i, int type) {
    switch (type) {
        default:
            i->hidden = false;
            i->used = false;
            i->spec = NULL;
            break;
    }
}

item* generate_item(int x, int y, int type, int display) {
    item* i = create_item(x, y, type, display);
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
    //? The free of the specs is to be handled separately
    free(i);
}