#ifndef GENERATION_H
#define GENERATION_H

#include "hash.h"
#include "parser.h"

/// @brief Gate position/type
enum Direction {  //! DO NOT MODIFY
    STARGATE,
    EAST,
    NORTH,
    WEST,
    SOUTH
};

typedef struct chunk chunk;
/// @brief define link to an array of chunk*
typedef chunk** chunk_link;

/// @brief Chunk
typedef struct chunk {
    chunk_link link;
    int x;
    int y;
    int type;
    dynarray* elements;
    hm* hashmap;
} chunk;

/// @brief Gives a random type (w/ exceptions) and add furnitures.
/// @param c raw chunk
/// @param x pos x
/// @param y pos y
void decorate(chunk* c, int x, int y);

/// @brief Return a dynarray of chunk decoration
/// @param ck chunk
/// @return dynarray of decoration
dynarray* get_chunk_furniture_list(chunk* ck);

/// @brief Return a hashmap containing the chunk decorations (key: x, y)
/// @param ck chunk
/// @return hashmap
hm* get_chunk_furniture_coords(chunk* ck);

/// @brief Get chunk x
/// @param c chunk
/// @return x
int get_chunk_x(chunk* c);

/// @brief Get chunk y
/// @param c chunk
/// @return y
int get_chunk_y(chunk* c);

/// @brief Change the chunk type
/// @param c chunk
/// @param type new type
void set_chunk_type(chunk* c, int type);

void remove_item(chunk* ck, item* i);

#endif