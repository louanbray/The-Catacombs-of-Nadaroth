
#ifndef RENDER_H
#define RENDER_H

#include <stdio.h>
#include <wchar.h>

/// @brief Inventory
typedef struct hotbar hotbar;
/// @brief Dynarray
typedef struct dynarray dynarray;
/// @brief Player
typedef struct player player;
/// @brief Chunk
typedef struct chunk chunk;
/// @brief Map
typedef struct map map;

/// @brief Screen
typedef wchar_t** board;

/// @brief Returns a white screen of const size
/// @return board
board new_screen();

/// @brief Clear the board
/// @param b board
void white_screen(board b);

/// @brief Modify the board to display chunk elements
/// @param b board
/// @param d chunk decoration
void render_elements(board b, dynarray* d);

/// @brief Modify the board to display the chunk (depending on the type)
/// @param b board
/// @param c chunk to display
void render_chunk(board b, chunk* c);

/// @brief Modify the board to display the player (if he moved, delete last pos)
/// @param b board
/// @param p player
void render_player(board b, player* p);

/// @brief Render hotbar
/// @param b board
/// @param h hotbar
void render_hotbar(board b, hotbar* h);

/// @brief Render the given map (chunk -> elements -> player)
/// @param b board
/// @param map map
void render(board b, map* map);

/// @brief Clear the output and print the board
/// @param b board
void update_screen(board b);

#endif
