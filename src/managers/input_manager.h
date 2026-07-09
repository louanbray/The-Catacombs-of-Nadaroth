#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H
#include <ctype.h>
#include <locale.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#define usleep(us) Sleep((us) / 1000 + ((us) % 1000 != 0 ? 1 : 0))
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

#define KEY_PRESSED(key) (get_key_state((unsigned char)(key)))                                                  //! [/!\ RESETS ONLY AFTER OTHER KEY EVENT]
#define USE_KEY(key) (get_key_state((unsigned char)(key)) ? (release_key((unsigned char)(key)), true) : false)  //! ['EAT' THE KEY, SIMULATE A RELEASE]
typedef struct Render_Buffer Render_Buffer;
typedef struct player player;
typedef struct InputThreadArgs {
    player** p;
    Render_Buffer* screen;
    void (*mouse_left_event_callback)(Render_Buffer* screen, player* p, int x, int y);
    void (*mouse_right_event_callback)(Render_Buffer* screen, player* p, int x, int y);
    void (*mouse_scroll_callback)(Render_Buffer* screen, player* p, int x, int y, int direction);
    void (*printable_char_callback)(Render_Buffer* screen, player* p, int c);
} InputThreadArgs;
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
 * @param mouse_left_event_callback Callback function to handle left mouse button events. It takes the screen pointer, user-defined data pointer,
 *                             and the x and y coordinates of the mouse event.
 * @param mouse_right_event_callback Callback function to handle right mouse button events. It takes the screen pointer and user-defined data pointer.
 * @param mouse_scroll_callback Callback function to handle mouse scroll events. It takes the screen pointer, user-defined data pointer,
 *                              x and y coordinates, and scroll direction (1 for up, -1 for down).
 * @param printable_char_callback Callback function to handle printable character events. It takes the screen pointer, user-defined data pointer,
 *                                and the printable character.
 */
void process_input(player** p, Render_Buffer* screen,
                   void (*mouse_left_event_callback)(Render_Buffer* screen, player* p, int x, int y),
                   void (*mouse_right_event_callback)(Render_Buffer* screen, player* p, int x, int y),
                   void (*mouse_scroll_callback)(Render_Buffer* screen, player* p, int x, int y, int direction),
                   void (*printable_char_callback)(Render_Buffer* screen, player* p, int c));
/**
 * @brief Checks if a specific key is currently pressed.
 *
 * @param key The key to check the state of.
 * @return true if the key is currently pressed, false otherwise.
 */
bool get_key_state(unsigned char key);
/**
 * @brief Releases the specified key.
 *
 * This function is called to release a key that was previously pressed.
 *
 * @param key The key to be released. This is an unsigned char representing the key.
 */
void release_key(unsigned char key);
/**
 * @brief Locks the inputs by setting the unlock flag to false.
 */
void lock_inputs();
/**
 * @brief Unlocks the inputs by setting the unlock flag to true.
 */
void unlock_inputs();

/**
 * @brief Restore terminal mode
 */
void restore_terminal_mode();
#endif