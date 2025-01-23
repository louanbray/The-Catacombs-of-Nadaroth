#ifndef ENTITY_H
#define ENTITY_H

#include "dynarray.h"
#include "generation.h"
#include "item.h"

/**
 * @brief Creates a new entity with the given brain.
 *
 * @param brain A pointer to the brain (item) of the entity.
 * @return A pointer to the newly created entity.
 */
entity* create_entity(item* brain, chunk* c);

/**
 * @brief Adds a part (item) to the entity.
 *
 * @param entity A pointer to the entity.
 * @param part A pointer to the item to be added as a part of the entity.
 */
void add_entity_part(entity* entity, item* part);

/**
 * @brief Applies a function to each part (item) of the entity.
 *
 * @param entity A pointer to the entity.
 * @param f A function pointer that takes an item pointer and returns void.
 */
void for_each_entity_part(entity* entity, void (*f)(item*));

/**
 * @brief Destroys the entity and frees associated memory.
 *
 * @param entity A pointer to the entity to be destroyed.
 */
void destroy_entity(entity* entity);

/**
 * @brief Retrieves the brain of the given entity.
 *
 * This function returns the brain component of the specified entity.
 *
 * @param entity A pointer to the entity whose brain is to be retrieved.
 * @return The brain component of the entity.
 */
item* get_entity_brain(entity* entity);

/**
 * @brief Retrieves the parts of the given entity.
 *
 * This function returns the parts component of the specified entity.
 *
 * @param entity A pointer to the entity whose parts are to be retrieved.
 * @return The parts component of the entity.
 */
dynarray* get_entity_parts(entity* entity);

/**
 * @brief Destroys the entity and frees associated memory from the chunk.
 *
 * @param entity A pointer to the entity to be destroyed.
 */
void destroy_entity_from_chunk(entity* entity);

/// @brief Unlink all entity parts from the chunk but keep the entity
/// @param entity entity
void remove_entity_from_chunk(entity* entity);

/// @brief Move the entity of 1 unit in a chunk following the direction (if possible)
/// @param entity entity
/// @param dir direction
void move_entity(entity* entity, Direction dir);

#endif