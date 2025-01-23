
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
typedef struct Render_Buffer Render_Buffer;

/// @brief Returns a white screen of const size
/// @return board
Render_Buffer* create_screen();

/// @brief Clear the board to the default board
/// @param board board
void default_screen(board board);

/// @brief Clear the board
/// @param board board
void blank_screen(board board);

/// @brief Get the board from the render buffer
/// @param screen Render_Buffer
/// @return board
board get_board(Render_Buffer* screen);

/// @brief Render a char on the board
/// @param board board
/// @param x x
/// @param y y
/// @param character char
void render_char(board board, int x, int y, int character);

/// @brief Modify the board to display the chunk (depending on the type)
/// @param screen Render_Buffer
/// @param chunk chunk to display
void render_chunk(Render_Buffer* screen, chunk* chunk);

/// @brief Modify the board to display the player (if he moved, delete last pos)
/// @param screen Render_Buffer
/// @param player player
void render_player(Render_Buffer* screen, player* player);

/// @brief Render hotbar
/// @param screen Render_Buffer
/// @param hotbar hotbar
void render_hotbar(Render_Buffer* screen, hotbar* hotbar);

/// @brief Render player health
/// @param screen Render_Buffer
/// @param player player
void render_health(Render_Buffer* screen, player* player);

/// @brief Render the given map (chunk -> elements -> player)
/// @param screen Render_Buffer
/// @param map map
void render(Render_Buffer* screen, map* map);

/// @brief Render the current chunk (chunk -> elements -> player)
/// @param screen Render_Buffer
/// @param player player
void render_from_player(Render_Buffer* screen, player* player);

/// @brief Clear the output and print the board
/// @param screen Render_Buffer
void update_screen(Render_Buffer* screen);

/// @brief Display the board read on the given file //! MAYBE LATER DO A PRELOADING OF THE ASSETS TO PREVENT I/O SATURATION
/// @param screen Render_Buffer
/// @param filename the file you want to display the content of
void display_interface(Render_Buffer* screen, const char* filename);

/// @brief Display the cinematic read on the given file
/// @param screen Render_Buffer
/// @param filename the file you want to display the content of
/// @param delay the delay between each frame
void play_cinematic(Render_Buffer* screen, const char* filename, int delay);

#endif
