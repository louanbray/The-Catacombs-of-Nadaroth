
#ifndef ITEMS_H
#define ITEMS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/// @brief Item
typedef struct item item;

/// @brief item type
enum ItemType {  //? MODIFY to add different types of items
    WALL,
    GATE,
    SGATE,
    PICKABLE,
};

/// @brief Create item using given parameters with the specs based on his type
/// @param x pos x
/// @param y pos y
/// @param type Type
/// @return item
item* generate_item(int x, int y, int type, int display, int index);

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

int get_item_index(item* i);

void set_item_index(item* i, int index);

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
