#ifndef GENERATION_H
#define GENERATION_H

#include "hash.h"
#include "parser.h"

typedef struct chunk chunk;
/// @brief define link to an array of chunk*
typedef chunk** chunk_link;

/// @brief Chunk
typedef struct chunk {
    chunk_link link;
    int x, spawn_x;
    int y, spawn_y;
    ChunkType type;
    dynarray* elements;
    dynarray* enemies;
    hm* hashmap;
} chunk;

/// @brief Gives a random type (w/ exceptions) and add furniture.
/// @param c raw chunk
/// @param x pos x
/// @param y pos y
void decorate(chunk* chunk, int x, int y);

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

/// @brief Get the links of the chunk
/// @param chunk chunk
/// @return chunk_link (array of 5 chunk pointers)
chunk_link get_chunk_links(chunk* chunk);

#endif