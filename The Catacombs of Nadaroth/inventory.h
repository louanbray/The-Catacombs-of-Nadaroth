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
/// @param hotbar hotbar
/// @param index index
/// @param item item
void set_hotbar(hotbar* hotbar, int index, item* item);

/// @brief Get hotbar[index]
/// @param hotbar hotbar
/// @param index index
/// @return item
item* get_hotbar(hotbar* hotbar, int index);

/// @brief Returns index of selected slot
/// @param hotbar hotbar
/// @return index
int get_selected_slot(hotbar* hotbar);

/// @brief Returns item in selected slot
/// @param hotbar hotbar
/// @return item
item* get_selected_item(hotbar* hotbar);

/// @brief Get hotbar max size
/// @param hotbar  hotbar
/// @return maxsize
int get_hotbar_max_size(hotbar* hotbar);

/// @brief Returns if the hotbar is full
/// @param hotbar hotbar
/// @return true if full
bool is_hotbar_full(hotbar* hotbar);

/// @brief Pickup an item, set to first free spot
/// @param hotbar hotbar
/// @param item item
void pickup(hotbar* hotbar, item* item);

/// @brief Drop(DESTROY ?) the item at index
/// @param hotbar hotbar
/// @param index index
void drop(hotbar* hotbar, int index);

/// @brief Select hotbar slot
/// @param hotbar hotbar
/// @param index index
void select_slot(hotbar* hotbar, int index);

/// @brief Get current number of items in hotbar
/// @param hotbar hotbar
/// @return number of items
int get_hotbar_entries(hotbar* hotbar);

#endif