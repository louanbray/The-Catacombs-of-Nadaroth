#ifndef PARSER_H
#define PARSER_H

#include "../game_objects/item.h"
#include "../utils/dynarray.h"

typedef struct chunk chunk;

/// @brief Get and add the chunk decoration based on its type
/// @param chunk chunk
/// @param array Array of items/decorations
/// @param type chunk type
void parse_chunk(chunk* chunk, dynarray* array, ChunkType type);

/// @brief Populate only static walls into chunk wall matrix
/// @param chunk chunk
/// @param type chunk type
void parse_chunk_walls(chunk* chunk, ChunkType type);

#endif