#ifndef PARSER_H
#define PARSER_H

#include "dynarray.h"
#include "item.h"

/// @brief chunk type (0,0) -> SPAWN
enum ChunkType {
    SPAWN,
    DEFAULT,
    DEFAULT2,
};

/// @brief Get and add the chunk decoration based on its type
/// @param d Array of items/decorations
/// @param type chunk type
void parse_chunk(dynarray* d, int type);

#endif