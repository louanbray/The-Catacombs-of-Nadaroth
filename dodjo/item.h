
#ifndef ITEMS_H
#define ITEMS_H

#include <stdbool.h>
#include <stdlib.h>

/// @brief Item
typedef struct item item;

/// @brief Create item using given parameters
/// @param type type of item
/// @param hidden Is hidden ?
/// @param used Is used ?
/// @param display Display char
/// @param spec Array of specs (different for each type | not handled here)
/// @return item
item* create_item(int x, int y, int type, bool hidden, bool used, int display, void* spec);

/// @brief Get item x
/// @param i item
/// @return pos x
int get_item_x(item* i);

/// @brief Get item y
/// @param i item
/// @return pos y
int get_item_y(item* i);

/// @brief Get item type
/// @param i item
/// @return type
int get_item_type(item* i);

/// @brief Get item display char
/// @param i item
/// @return display char
int get_item_display(item* i);

/// @brief Get item specs
/// @param i item
/// @return spec array
void* get_item_spec(item* i);

/// @brief Is item hidden (true if yes)
/// @param i item
/// @return bool is_hidden?
bool is_item_hidden(item* i);

/// @brief Is item used (true if yes)
/// @param i item
/// @return bool is_used?
bool is_item_used(item* i);

/// @brief Set hidden state to given parameter
/// @param i item
/// @param hidden bool
void set_item_hidden(item* i, bool hidden);

/// @brief Set used state to given parameter
/// @param i item
/// @param used bool
void set_item_used(item* i, bool used);

/// @brief Set display to given parameter
/// @param i item
/// @param display char
void set_item_display(item* i, int display);

/// @brief Set spec to given parameter
/// @param i item
/// @param spec void*
void set_item_spec(item* i, void* spec);

/// @brief Free item
/// @param i item
void free_item(item* i);

#endif
