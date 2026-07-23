#include "item.h"

#include "../game_objects/chunk.h"
#include "../managers/assets_manager.h"
typedef struct item {
    int x, y;
    ItemType type;
    bool hidden, used, is_arena;
    int display, index;
    void* spec;
    entity* entity_link;
    UsableItem usable_item;
    Color color;
} item;

item* generate_item_arena(chunk_arena* arena, int x, int y, ItemType type, int display, UsableItem usable_item, int index) {
    item* i = arena ? (item*)chunk_arena_alloc(arena, sizeof(item)) : (item*)malloc(sizeof(item));
    i->x = x;
    i->y = y;
    i->type = type;
    i->display = display;
    i->index = index;
    i->hidden = false;
    i->used = false;
    i->is_arena = (arena != NULL);
    i->spec = NULL;
    i->entity_link = NULL;
    i->usable_item = usable_item;
    i->color = COLOR_DEFAULT;
    return i;
}

item* generate_item(int x, int y, ItemType type, int display, UsableItem usable_item, int index) {
    return generate_item_arena(NULL, x, y, type, display, usable_item, index);
}

void specialize(item* i, bool used, bool hidden, void* spec) {
    i->spec = spec;
    i->used = used;
    i->hidden = hidden;
}

bool is_in_box(int x, int y) {
    return (y <= PLAYBOX_MAX_OY && y >= PLAYBOX_MIN_OY) && (x <= PLAYBOX_MAX_OX && x >= PLAYBOX_MIN_OX);
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

Rarity get_usable_item_rarity(UsableItem uitem) {
    UsableItemAssetFile* uif = get_usable_item_file(uitem);
    if (uif == NULL || uif->specs.spec_count == 0) return RARITY_NONE;
    return uif->specs.specs[0];
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

void set_item_index(item* i, int index) {
    i->index = index;
}

void free_item(item* i) {
    if (i == NULL) return;
    if (!i->is_arena) {
        if (i->spec != NULL) {
            free(i->spec);
        }
        free(i);
    }
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