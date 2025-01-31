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

#define KEY_PRESSED(key) (get_key_state((unsigned char)(key)))

typedef struct Render_Buffer Render_Buffer;
typedef struct player player;

/**
 * @brief Initializes the terminal settings for the application.
 *
 * This function sets up the terminal environment to ensure that the application
 * can properly handle input and output operations. It may configure terminal
 * modes, clear the screen, or perform other necessary initialization tasks.
 */
void init_terminal();

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

/**
 * @brief Checks if a specific key is currently pressed.
 *
 * @param key The key to check the state of.
 * @return true if the key is currently pressed [/!\ RESETS ONLY AFTER OTHER KEY EVENT], false otherwise.
 */
bool get_key_state(unsigned char key);

/**
 * @brief Locks the inputs by setting the unlock flag to false.
 */
void lock_inputs();

/**
 * @brief Unlocks the inputs by setting the unlock flag to true.
 */
void unlock_inputs();

#endif