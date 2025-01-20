#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <ctype.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>

typedef struct Render_Buffer Render_Buffer;
typedef struct player player;

/// @brief Setup the terminal for display and inputs
void init_terminal();

/**
 * @brief Parses an SGR mouse event from the given buffer.
 *
 * @param buffer The buffer containing the SGR mouse event data.
 * @param target_x Pointer to an integer where the x-coordinate of the mouse event will be stored.
 * @param target_y Pointer to an integer where the y-coordinate of the mouse event will be stored.
 * @param left_click Pointer to an integer that will be set to 1 if the left mouse button was clicked, otherwise 0.
 * @param just_pressed Pointer to a boolean that indicates whether the left mouse button was just pressed.
 */
void parse_sgr_mouse_event(const char* buffer, int* target_x, int* target_y, int* left_click, bool* just_pressed);

/**
 * @brief Checks if the given buffer contains a mouse event.
 *
 * @param buffer The buffer to check.
 * @param length The length of the buffer.
 * @return true if the buffer contains a mouse event, false otherwise.
 */
bool is_mouse_event(const char* buffer, size_t length);

/**
 * @brief Checks if the given buffer contains an arrow key event.
 *
 * @param buffer The buffer to check.
 * @param length The length of the buffer.
 * @return true if the buffer contains an arrow key event, false otherwise.
 */
bool is_arrow_key(const char* buffer, size_t length);

/**
 * @brief Gets the length of a mouse event in the given buffer.
 *
 * @param buffer The buffer containing the mouse event data.
 * @param length The length of the buffer.
 * @return The length of the mouse event, or 0 if the event is incomplete.
 */
size_t get_mouse_event_length(const char* buffer, size_t length);

/**
 * @brief Processes input from the standard input and handles mouse events, arrow keys, and printable characters.
 *
 * This function continuously reads input from the standard input and processes it to handle mouse events, arrow keys,
 * and printable characters. It uses the provided callback functions to handle each type of input event.
 *
 * @param p Pointer to user-defined data that will be passed to the callback functions.
 * @param screen Pointer to the screen or context that will be passed to the callback functions.
 * @param mouse_event_callback Callback function to handle mouse events. It takes the screen pointer, user-defined data pointer,
 *                             and the x and y coordinates of the mouse event.
 * @param arrow_key_callback Callback function to handle arrow key events. It takes the screen pointer, user-defined data pointer,
 *                           and the arrow key character.
 * @param printable_char_callback Callback function to handle printable character events. It takes the screen pointer, user-defined data pointer,
 *                                and the printable character.
 */
void process_input(player* p, Render_Buffer* screen,
                   void (*mouse_event_callback)(Render_Buffer* screen, player* p, int x, int y),
                   void (*arrow_key_callback)(Render_Buffer* screen, player* p, int arrow_key),
                   void (*printable_char_callback)(Render_Buffer* screen, player* p, int c));

#endif