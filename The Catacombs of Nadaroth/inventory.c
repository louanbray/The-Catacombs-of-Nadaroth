#include "inventory.h"

#include "item.h"

/// @brief Hotbar
typedef struct hotbar {
    item** items;
    int selected, entries;
    item* selected_item;
    int max_size;
} hotbar;

const int HOTBAR_SIZE = 9;

hotbar* create_hotbar() {
    hotbar* h = malloc(sizeof(hotbar));
    item** i = calloc(9, sizeof(item*));
    h->items = i;
    h->selected = 0;
    h->entries = 0;
    h->selected_item = NULL;
    h->max_size = HOTBAR_SIZE;
    return h;
}

void set_hotbar(hotbar* h, int index, item* e) {
    h->items[index] = e;
}

item* get_hotbar(hotbar* h, int index) {
    return h->items[index];
}

int get_selected_slot(hotbar* h) {
    return h->selected;
}

item* get_selected_item(hotbar* h) {
    return h->selected_item;
}

int get_hotbar_max_size(hotbar* h) {
    return h->max_size;
}

bool is_hotbar_full(hotbar* h) {
    return h->entries == HOTBAR_SIZE;
}

void pickup(hotbar* h, item* e) {
    if (is_hotbar_full(h)) {
        return;
    }

    int i = 0;

    for (i = 0; i < HOTBAR_SIZE; i++) {
        if (h->items[i] == NULL) {
            break;
        }
    }

    h->items[i] = e;

    if (i == h->selected) {
        h->selected_item = e;
    }

    h->entries += 1;
}

void drop(hotbar* h, int index) {
    if (h->items[index] == NULL) return;

    free_item(h->items[index]);

    h->items[index] = NULL;

    if (h->selected == index) {
        h->selected_item = NULL;
    }

    h->entries += -1;
}

void select_slot(hotbar* h, int index) {
    if (index != h->selected) {
        h->selected = index;
        h->selected_item = get_hotbar(h, index);
    }
}