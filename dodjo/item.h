
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

/**
 * @brief Get the index of the given item.
 *
 * @param i Pointer to the item.
 * @return int The index of the item.
 */
int get_item_index(item* i);

/**
 * @brief Set the index of the given item.
 *
 * @param i Pointer to the item.
 * @param index The index to set for the item.
 */
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

/// @brief Complete the item specs depending on its type
/// @param i item
/// @param type type
void specialize(item* i, bool used, bool hidden, void* spec);

/// @brief Link an entity to an item
/// @param i item
/// @param e entity
void link_entity(item* i, entity* e);

/**
 * @brief Checks if the given item is an entity.
 *
 * This function determines whether the provided item has an associated entity link.
 *
 * @param i A pointer to the item to be checked.
 * @return true if the item has an entity link, false otherwise.
 */
bool is_an_entity(item* i);

/**
 * @brief Retrieves the entity link of the given item.
 *
 * This function returns the entity link associated with the provided item.
 *
 * @param i A pointer to the item whose entity link is to be retrieved.
 * @return A pointer to the entity link if it exists, otherwise NULL.
 */
entity* get_entity_link(item* i);

#endif
