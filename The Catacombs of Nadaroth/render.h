#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <stdio.h>
#include <wchar.h>

#include "constants.h"  // Adjust as needed for your project

// Forward declarations for types used elsewhere
typedef struct hotbar hotbar;
typedef struct dynarray dynarray;
typedef struct player player;
typedef struct chunk chunk;
typedef struct map map;

// Color enumeration (using a small integer type)
typedef enum Color {
    COLOR_DEFAULT = 0,
    COLOR_RED,
    COLOR_CYAN_BOLD,
    COLOR_YELLOW,
    COLOR_MAGENTA_BOLD,
    // Extend as needed
} Color;

// Each cell now stores a wide character and a color.
typedef struct Cell Cell;

// Our board is now a 2D array of Cells.
typedef Cell** board;

// Render Buffer
typedef struct Render_Buffer Render_Buffer;

//
// Function Prototypes
//

/// @brief Creates and initializes a new Render_Buffer.
/// @return Pointer to the new Render_Buffer.
Render_Buffer* create_screen();

/// @brief Clears the board to the default board (draws borders and decorations).
/// @param screen the screen.
void default_screen(board screen);

/// @brief Clears the board (fills with blank/default cells).
/// @param b The board to clear.
void blank_screen(board b);

/// @brief Returns the current board from the render buffer.
/// @param screen Pointer to the Render_Buffer.
/// @return The current board.
board get_board(Render_Buffer* screen);

/// @brief Renders a character onto the board at center-based coordinates.
/// @param screen The board to render on.
/// @param x The x coordinate (center based).
/// @param y The y coordinate (center based).
/// @param character The character to render.
/// @param rb Pointer to the Render_Buffer (for dirty marking).
void render_char(board rb, int x, int y, int character);

/// @brief Renders a narrow (ASCII) string onto the render buffer.
/// @param screen Pointer to the Render_Buffer.
/// @param x The x coordinate (center based).
/// @param y The y coordinate (center based).
/// @param s The string to render.
/// @param len The number of characters to render.
void render_string(Render_Buffer* screen, int x, int y, char* s, int len);

/// @brief Renders a wide string onto the render buffer.
/// @param screen Pointer to the Render_Buffer.
/// @param x The x coordinate (center based).
/// @param y The y coordinate (center based).
/// @param s The wide string to render.
/// @param len The number of characters to render.
void render_unicode_string(Render_Buffer* screen, int x, int y, wchar_t* s, int len);

/// @brief Renders the title of an item on the screen.
/// @param it Pointer to the item.
void render_item_title(Render_Buffer* screen, void* it);

/// @brief Updates the board to display the chunk (e.g., furniture, chunk info).
/// @param screen Pointer to the Render_Buffer.
/// @param chunk Pointer to the chunk to display.
void render_chunk(Render_Buffer* screen, chunk* chunk);

/// @brief Renders the player on the board (clearing the previous position if needed).
/// @param screen Pointer to the Render_Buffer.
/// @param player Pointer to the player.
void render_player(Render_Buffer* screen, player* player);

/// @brief Renders the hotbar on the board.
/// @param screen Pointer to the Render_Buffer.
/// @param hotbar Pointer to the hotbar.
void render_hotbar(Render_Buffer* screen, hotbar* hotbar);

/// @brief Renders the player's health on the board.
/// @param screen Pointer to the Render_Buffer.
/// @param player Pointer to the player.
void render_health(Render_Buffer* screen, player* player);

/// @brief Renders the entire map (chunk, elements, player) onto the board.
/// @param screen Pointer to the Render_Buffer.
/// @param map Pointer to the map.
void render(Render_Buffer* screen, map* map);

/// @brief Renders the current chunk (elements and player) based on the player's perspective.
/// @param screen Pointer to the Render_Buffer.
/// @param player Pointer to the player.
void render_from_player(Render_Buffer* screen, player* player);

/// @brief Clears the output and prints the board (using dirty region optimization).
/// @param screen Pointer to the Render_Buffer.
void update_screen(Render_Buffer* screen);

/// @brief Displays the description of the given item in a modal view.
/// @param r Pointer to the Render_Buffer.
/// @param it Pointer to the item.
void display_item_description(Render_Buffer* r, void* it);

/// @brief Displays an interface read from a file (e.g., help screen).
/// @param screen Pointer to the Render_Buffer.
/// @param filename Name of the file to display.
void display_interface(Render_Buffer* screen, const char* filename);

/// @brief Plays a cinematic from a file.
/// @param screen Pointer to the Render_Buffer.
/// @param filename Name of the file containing the cinematic frames.
/// @param delay Delay between frames.
void play_cinematic(Render_Buffer* screen, const char* filename, int delay);

/// @brief Returns the character stored in the cell at (row, col) in the current board.
/// @param r Pointer to the Render_Buffer
/// @param row char row
/// @param col char col
/// @return char at these coordinates
wchar_t render_get_cell_char(Render_Buffer* r, int row, int col);

#endif
