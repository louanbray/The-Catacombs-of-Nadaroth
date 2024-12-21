#ifndef INVENTORY_H
#define INVENTORY_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct item item;
typedef struct hotbar hotbar;

/// @brief Create a new hotbar
/// @return hotbar
hotbar* create_hotbar();

/// @brief Set hotbar[index] to e
/// @param h hotbar
/// @param index index
/// @param e item
void set_hotbar(hotbar* h, int index, item* e);

/// @brief Get hotbar[index]
/// @param h hotbar
/// @param index index
/// @return item
item* get_hotbar(hotbar* h, int index);

/// @brief Return index of selected slot
/// @param h hotbar
/// @return index
int get_selected_slot(hotbar* h);

/// @brief Return item in selected slot
/// @param h hotbar
/// @return item
item* get_selected_item(hotbar* h);

/// @brief Get hotbar max size
/// @param h  hotbar
/// @return maxsize
int get_hotbar_max_size(hotbar* h);

/// @brief Return if the hotbar is full
/// @param h hotbar
/// @return true if full
bool is_hotbar_full(hotbar* h);

/// @brief Pickup an item, set to first free spot
/// @param h hotbar
/// @param e item
void pickup(hotbar* h, item* e);

/// @brief Drop(DESTROY ?) the item at index
/// @param h hotbar
/// @param index index
void drop(hotbar* h, int index);

/// @brief Select hotbar slot
/// @param h hotbar
/// @param index index
void select_slot(hotbar* h, int index);

#endif