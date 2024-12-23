
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
/// @brief Render Buffer
typedef struct renderbuffer renderbuffer;

/// @brief Returns a white screen of const size
/// @return board
renderbuffer* create_screen();

/// @brief Clear the board to the default board
/// @param b board
void default_screen(board b);

/// @brief Clear the board
/// @param b board
void blank_screen(board b);

/// @brief Get the board from the render buffer
/// @param r render buffer
/// @return board
board get_board(renderbuffer* r);

/// @brief Modify the board to display the chunk (depending on the type)
/// @param b board
/// @param c chunk to display
void render_chunk(renderbuffer* r, chunk* c);

/// @brief Modify the board to display the player (if he moved, delete last pos)
/// @param b board
/// @param p player
void render_player(renderbuffer* r, player* p);

/// @brief Render hotbar
/// @param b board
/// @param h hotbar
void render_hotbar(renderbuffer* r, hotbar* h);

/// @brief Render the given map (chunk -> elements -> player)
/// @param b board
/// @param map map
void render(renderbuffer* r, map* map);

/// @brief Render the current chunk (chunk -> elements -> player)
/// @param b board
/// @param player player
void render_from_player(renderbuffer* r, player* p);

/// @brief Clear the output and print the board
/// @param b board
void update_screen(renderbuffer* r);

#endif
