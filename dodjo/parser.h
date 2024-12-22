#ifndef PARSER_H
#define PARSER_H

#include "dynarray.h"
#include "item.h"

/// @brief chunk type (0,0) -> SPAWN
enum ChunkType {
    SPAWN,
    DEFAULT
};

void parse_chunk(dynarray* d, int type);
void parse_item(item* i, int type);

#endif