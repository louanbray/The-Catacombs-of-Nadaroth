#ifndef PARSER_H
#define PARSER_H

#include "dynarray.h"
#include "item.h"

typedef struct chunk chunk;

/// @brief Get and add the chunk decoration based on its type
/// @param chunk chunk
/// @param array Array of items/decorations
/// @param type chunk type
void parse_chunk(chunk* chunk, dynarray* array, enum ChunkType type);

// TODO: IMPLEMENT THE FOLLOWING METHODS

/*
void parse_item(...)
*/

#endif