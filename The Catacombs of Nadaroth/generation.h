#ifndef GENERATION_H
#define GENERATION_H

#include "hash.h"
#include "parser.h"

typedef struct chunk chunk;
/// @brief define link to an array of chunk*
typedef chunk** chunk_link;

// ---- Decoration ----

/// @brief Gives a random type (w/ exceptions) and add furniture.
/// @param c raw chunk
/// @param x pos x
/// @param y pos y
void decorate(chunk* chunk, int x, int y);

// ---- Accessors ----

/// @brief Returns a dynarray of chunk decoration
/// @param chunk chunk
/// @return dynarray of decoration
dynarray* get_chunk_furniture_list(chunk* chunk);

/// @brief Returns a hashmap containing the chunk decorations (key: x, y)
/// @param chunk chunk
/// @return hashmap
hm* get_chunk_furniture_coords(chunk* chunk);

/// @brief Get chunk x
/// @param chunk chunk
/// @return x
int get_chunk_x(chunk* chunk);

/// @brief Get chunk y
/// @param chunk chunk
/// @return y
int get_chunk_y(chunk* chunk);

/// @brief Get chunk spawn pos x
/// @param chunk chunk
/// @return spawn x
int get_chunk_spawn_x(chunk* chunk);

/// @brief Get chunk spawn pos y
/// @param chunk chunk
/// @return spawn y
int get_chunk_spawn_y(chunk* chunk);

/// @brief Change the chunk type
/// @param chunk chunk
/// @param type new type
void set_chunk_type(chunk* chunk, ChunkType type);

/// @brief Remove all elements from the chunk but doesn't destroy the item
/// @param chunk
/// @param item
void remove_item(chunk* chunk, item* item);

/// @brief Get the enemies of the chunk
/// @param chunk
/// @return dynarray of enemies
dynarray* get_chunk_enemies(chunk* chunk);

/// @brief Get the type of the chunk
/// @param chunk chunk
/// @return ChunkType
ChunkType get_chunk_type(chunk* chunk);

/// @brief Get the links of the chunk (array of 5 chunk pointers)
/// @param chunk chunk
/// @return chunk_link
chunk_link get_chunk_links(chunk* chunk);

/// @brief Get the linked chunk at direction index [0..4]
/// @param ck chunk
/// @param dir direction index
/// @return linked chunk or NULL
chunk* get_chunk_link_at(chunk* ck, int dir);

/// @brief Set the linked chunk at direction index [0..4]
/// @param ck chunk
/// @param dir direction index
/// @param target target chunk (may be NULL)
void set_chunk_link(chunk* ck, int dir, chunk* target);

// ---- Element / enemy helpers ----

/// @brief Append an item to the chunk's elements array
/// @param ck chunk
/// @param it item (may be NULL)
void chunk_append_element(chunk* ck, item* it);

/// @brief Register an item's coordinates in the chunk's hashmap
/// @param ck chunk
/// @param it item (non-NULL)
void chunk_register_item(chunk* ck, item* it);

/// @brief Append an item to the chunk's enemies array
/// @param ck chunk
/// @param it item (may be NULL)
void chunk_append_enemy(chunk* ck, item* it);

/// @brief Get the number of items in the elements array
/// @param ck chunk
/// @return count
int chunk_element_count(chunk* ck);

/// @brief Get the element at index i (may be NULL)
/// @param ck chunk
/// @param i index
/// @return item or NULL
item* chunk_get_element(chunk* ck, int i);

// ---- Lifecycle ----

/// @brief Create an empty raw chunk at (x, y) with SPAWN type
/// @param x chunk coord x
/// @param y chunk coord y
/// @return allocated chunk
chunk* create_chunk(int x, int y);

/// @brief Create and decorate a chunk at (x, y)
/// @param x chunk coord x
/// @param y chunk coord y
/// @return decorated chunk
chunk* generate_chunk(int x, int y);

/// @brief Create a chunk from deserialized data without decoration
/// @param x chunk coord x
/// @param y chunk coord y
/// @param spawn_x player spawn x inside chunk
/// @param spawn_y player spawn y inside chunk
/// @param type chunk type
/// @return allocated chunk
chunk* create_chunk_raw(int x, int y, int spawn_x, int spawn_y, ChunkType type);

/// @brief Free all items in elements, recreate internal arrays, update metadata
/// @param ck chunk to reset
/// @param spawn_x new spawn x
/// @param spawn_y new spawn y
/// @param type new type
void reset_chunk_internals(chunk* ck, int spawn_x, int spawn_y, ChunkType type);

/// @brief Free all internals and the chunk object itself
/// @param ck chunk to destroy
void destroy_chunk_full(chunk* ck);

#endif