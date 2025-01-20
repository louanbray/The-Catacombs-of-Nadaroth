
#ifndef ITEMS_H
#define ITEMS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @struct entity
 * @brief Represents an entity with parts and a brain.
 *
 * @var entity::parts
 * A dynamic array of parts (items) associated with the entity.
 *
 * @var entity::brain
 * A pointer to the brain (item) of the entity.
 */
typedef struct entity entity;

/// @brief Item
typedef struct item item;

/// @brief Type of item
typedef struct enemy {
    int hp;
    // int damage; //TODO: IMPLEMENT THIS
} enemy;

/// @brief item type
enum ItemType {  //? MODIFY to add different types of items
    WALL,
    GATE,
    SGATE,
    PICKABLE,
    ENEMY,
};

/// @brief Create item using given parameters with the specs based on his type
/// @param pos_x pos x
/// @param pos_y pos y
/// @param type Type
/// @return item
item* generate_item(int pos_x, int pos_y, enum ItemType type, int display, int index);

/// @brief Get item x
/// @param item item
/// @return pos x
int get_item_x(item* item);

/// @brief Get item y
/// @param item item
/// @return pos y
int get_item_y(item* item);

/// @brief Get item type
/// @param item item
/// @return type
int get_item_type(item* item);

/// @brief Get item display char
/// @param item item
/// @return display char
int get_item_display(item* item);

/// @brief Get item specs
/// @param item item
/// @return spec array
void* get_item_spec(item* item);

/**
 * @brief Get the index of the given item.
 *
 * @param item Pointer to the item.
 * @return int The index of the item.
 */
int get_item_index(item* item);

/**
 * @brief Set the index of the given item.
 *
 * @param item Pointer to the item.
 * @param index The index to set for the item.
 */
void set_item_index(item* item, int index);

/// @brief Is item hidden (true if yes)
/// @param item item
/// @return bool is_hidden?
bool is_item_hidden(item* item);

/// @brief Is item used (true if yes)
/// @param item item
/// @return bool is_used?
bool is_item_used(item* item);

/// @brief Set hidden state to given parameter
/// @param item item
/// @param hidden bool
void set_item_hidden(item* item, bool hidden);

/// @brief Set used state to given parameter
/// @param item item
/// @param used bool
void set_item_used(item* item, bool used);

/// @brief Set display to given parameter
/// @param item item
/// @param display char
void set_item_display(item* item, int display);

/// @brief Set spec to given parameter
/// @param item item
/// @param spec void*
void set_item_spec(item* item, void* spec);

/// @brief Set x to given parameter
/// @param item item
/// @param pos_x pos x
void set_item_x(item* item, int pos_x);

/// @brief Set y to given parameter
/// @param item item
/// @param pos_y pos y
void set_item_y(item* item, int pos_y);

/// @brief Free item
/// @param item item
void free_item(item* item);

/// @brief Complete the item specs depending on its type
/// @param item item
/// @param type type
void specialize(item* item, bool used, bool hidden, void* spec);

/// @brief Link an entity to an item
/// @param item item
/// @param entity entity
void link_entity(item* item, entity* entity);

/**
 * @brief Checks if the given item is an entity.
 *
 * This function determines whether the provided item has an associated entity link.
 *
 * @param item A pointer to the item to be checked.
 * @return true if the item has an entity link, false otherwise.
 */
bool is_an_entity(item* item);

/**
 * @brief Retrieves the entity link of the given item.
 *
 * This function returns the entity link associated with the provided item.
 *
 * @param item A pointer to the item whose entity link is to be retrieved.
 * @return A pointer to the entity link if it exists, otherwise NULL.
 */
entity* get_entity_link(item* item);

/// @brief Check if coords are in chunk bounds
/// @param pos_x pos x
/// @param pos_y pos y
/// @return true if in bounds
bool is_in_box(int pos_x, int pos_y);

#endif
