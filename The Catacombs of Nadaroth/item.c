#include "item.h"

typedef struct item {
    int x, y;
    ItemType type;
    bool hidden, used;
    int display, index;
    void* spec;
    entity* entity_link;
    UsableItem usable_item;
    Color color;
} item;

/// @brief Create item using given parameters
/// @param x pos x
/// @param y pos y
/// @param type Type
/// @return item
item* create_item(int x, int y, ItemType type, int display, UsableItem usable_item, int index) {
    item* i = malloc(sizeof(item));
    i->x = x;
    i->y = y;
    i->type = type;
    i->display = display;
    i->index = index;
    i->hidden = false;
    i->used = false;
    i->spec = NULL;
    i->entity_link = NULL;
    i->usable_item = usable_item;
    i->color = COLOR_DEFAULT;
    return i;
}

void specialize(item* i, bool used, bool hidden, void* spec) {
    i->spec = spec;
    i->used = used;
    i->hidden = hidden;
}

item* generate_item(int x, int y, ItemType type, int display, UsableItem usable_item, int index) {
    item* i = create_item(x, y, type, display, usable_item, index);
    return i;
}

bool is_in_box(int x, int y) {
    return (y <= 17 && y >= -17) && (x <= 63 && x >= -64);
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

void* get_item_spec(item* i) {
    return i->spec;
}

UsableItem get_item_usable_type(item* i) {
    return i->usable_item;
}

Color get_item_color(item* i) {
    return i->color;
}

bool is_item_hidden(item* i) {
    return i->hidden;
}

bool is_item_used(item* i) {
    return i->used;
}

void set_item_x(item* i, int x) {
    i->x = x;
}

void set_item_y(item* i, int y) {
    i->y = y;
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

void set_item_usable_type(item* i, UsableItem usable_item) {
    i->usable_item = usable_item;
}

void set_item_color(item* i, Color color) {
    i->color = color;
}

void free_item(item* i) {
    if (i->spec != NULL) {
        free(i->spec);
    }
    free(i);
}

void link_entity(item* i, entity* e) {
    i->entity_link = e;
}

bool is_an_entity(item* i) {
    return i->entity_link != NULL;
}

entity* get_entity_link(item* i) {
    return i->entity_link;
}