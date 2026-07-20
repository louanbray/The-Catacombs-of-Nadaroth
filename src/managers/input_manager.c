#include "input_manager.h"

#include <time.h>

#include "../utils/game_status.h"
#include "../utils/logger.h"
#include "../utils/sys_platform.h"

#define MAX_KEYS 256
#define MAX_BUFFER_SIZE 1024
#define CTRL_C 0x03  // CTRL+C character code

static bool key_states[MAX_KEYS] = {false};
static bool key_pressed_last_frame[MAX_KEYS] = {false};
static bool unlock = true;

#ifdef _WIN32
static HANDLE hStdin = NULL;
static DWORD original_console_mode = 0;

static BOOL WINAPI windows_ctrl_c_handler(DWORD fdwCtrlType) {
    if (fdwCtrlType == CTRL_C_EVENT || fdwCtrlType == CTRL_BREAK_EVENT || fdwCtrlType == CTRL_CLOSE_EVENT) {
        stop_game();  // On lève le même flag que ton code actuel
        return TRUE;  // On dit à Windows qu'on gère le signal nous-mêmes
    }
    return FALSE;
}
#else
static struct termios original_termios;  // Global to store original terminal settings
#endif

/// @brief Save the current terminal settings
void save_original_mode() {
#ifdef _WIN32
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE || !GetConsoleMode(hStdin, &original_console_mode)) {
        LOG_ERROR("GetConsoleMode failed (err %lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }
#else
    if (tcgetattr(STDIN_FILENO, &original_termios) < 0) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
#endif
}

/// @brief Restore the terminal to its original mode
void restore_terminal_mode() {
#ifdef _WIN32
    if (hStdin) {
        SetConsoleMode(hStdin, original_console_mode);
    }
#else
    if (tcsetattr(STDIN_FILENO, TCSANOW, &original_termios) < 0) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
#endif
    printf("\33[?25h");                // Re-enable cursor visibility
    printf("\033[?1003l\033[?1006l");  // Disable Mouse events
    printf("\033[?2004l");             // Disable bracketed paste mode
    printf("\033[?1049l");             // back to normal terminal
    printf("\033[H\033[2J");           // Clear the screen
    printf("\033[0m");                 // Reset color
    fflush(stdout);
}

/// @brief Signal handler to restore terminal on unexpected termination
void handle_signal(int signo) {
    (void)signo;
    restore_terminal_mode();
    exit(EXIT_FAILURE);
}

/// @brief Ensure restoration on program exit
void setup_terminal_restoration() {
    save_original_mode();
    atexit(restore_terminal_mode);  // Ensure restoration on normal exit
#ifdef _WIN32
    SetConsoleCtrlHandler(windows_ctrl_c_handler, TRUE);
#endif
    // Note: SIGINT is disabled at the terminal level (ISIG flag / raw mode)
    // so CTRL+C generates character 0x03 instead of a signal
    signal(SIGTERM, handle_signal);  // Handle termination signals
}

// Function to set the terminal to raw mode
void set_raw_mode() {
#ifdef _WIN32
    // Turn off line buffering / echo / ^C-as-signal, turn on VT input so
    // escape sequences (mouse, arrow keys, paste) arrive as raw bytes just
    // like on a POSIX terminal. Requires Windows Terminal (or any host with
    // VT input support) - legacy conhost has partial/buggy support for this.
    DWORD mode = original_console_mode;
    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
    mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
    if (!SetConsoleMode(hStdin, mode)) {
        LOG_ERROR("SetConsoleMode (input) failed (err %lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    // Also make sure stdout actually interprets the ANSI escape codes below
    // (older conhost needs this turned on explicitly; Windows Terminal
    // already defaults to it, but enabling it again is harmless).
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD out_mode = 0;
    if (hStdout != INVALID_HANDLE_VALUE && GetConsoleMode(hStdout, &out_mode)) {
        SetConsoleMode(hStdout, out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#else
    struct termios t;

    // Get the current terminal attributes
    if (tcgetattr(STDIN_FILENO, &t) < 0) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Modify the terminal attributes for raw mode
    t.c_lflag &= ~(ICANON | ECHO | ISIG);  // Disable canonical mode, echo, and signal generation
    t.c_iflag &= ~(IXON);                  // Disable flow control (CTRL+S/CTRL+Q)
    t.c_cc[VMIN] = 1;                      // Minimum number of characters to read
    t.c_cc[VTIME] = 0;                     // Timeout (deciseconds) for read

    tcsetattr(STDIN_FILENO, TCSANOW, &t);
#endif
}

#ifndef _WIN32
// Function to set the file descriptor to non-blocking mode (POSIX only -
// on Windows non-blocking behavior is handled inside read_stdin_nonblocking
// below instead, since fcntl(O_NONBLOCK) doesn't apply to console handles).
void set_nonblocking_mode(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}
#else
void set_nonblocking_mode(int fd) {
    (void)fd;  // no-op on Windows; see read_stdin_nonblocking()
}
#endif

/// @brief Reads available stdin bytes without blocking. Returns 0 if
/// nothing is available yet (mirrors what O_NONBLOCK + read() gives on
/// POSIX), or -1 on a real error.
static ssize_t read_stdin_nonblocking(char* buf, size_t size) {
#ifdef _WIN32
    DWORD waited = WaitForSingleObject(hStdin, 0);
    if (waited != WAIT_OBJECT_0) {
        return 0;  // nothing queued yet
    }
    DWORD bytes_read = 0;
    if (!ReadFile(hStdin, buf, (DWORD)size, &bytes_read, NULL)) {
        return -1;
    }
    return (ssize_t)bytes_read;
#else
    return read(STDIN_FILENO, buf, size);
#endif
}

#ifdef _WIN32
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

void init_terminal() {
    setup_terminal_restoration();

    set_raw_mode();
    set_nonblocking_mode(STDIN_FILENO);

    setlocale(LC_ALL, "");

#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
#endif

    printf("\033[?1049h");             // alternate buffer
    printf("\033[2J\033[H");           // clear
    printf("\33[?25l");                // Disable cursor
    printf("\033[?1003h\033[?1006h");  // Enable mouse motion events
    printf("\033[?2004h");             // Enable bracketed paste mode
    fflush(stdout);
}

typedef struct MouseEvent {
    int target_x;
    int target_y;
    bool left_just_pressed;
    bool right_just_pressed;
    int left_click;
    int right_click;
    int scroll_up;
    int scroll_down;
} MouseEvent;

/**
 * @brief Parses an SGR mouse event from the given buffer.
 *
 * @param buffer The buffer containing the SGR mouse event data.
 * @param target_x Pointer to an integer where the x-coordinate of the mouse event will be stored.
 * @param target_y Pointer to an integer where the y-coordinate of the mouse event will be stored.
 * @param left_just_pressed Pointer to a boolean that indicates whether the left mouse button was just pressed.
 * @param right_just_pressed Pointer to a boolean that indicates whether the right mouse button was just pressed.
 * @param left_click Pointer to an integer that is set to one after the first left click and then reset to 0 after the release (prevent holding)
 * @param right_click Pointer to an integer that is set to one after the first right click and then reset to 0 after the release (prevent holding)
 */
void parse_sgr_mouse_event(const char* buffer, MouseEvent* event) {
    int button, x, y;
    char event_type;

    event->left_click = 0;
    event->right_click = 0;
    event->scroll_up = 0;
    event->scroll_down = 0;

    if (sscanf(buffer, "\033[<%d;%d;%d%c", &button, &x, &y, &event_type) == 4) {
        // x -= 1;
        // y -= 1;
        // Check for scroll events (button 64 = scroll up, 65 = scroll down)
        if (button == 64) {
            event->scroll_up = 1;
            event->target_x = (x / 2) * 2;
            event->target_y = y;
            return;
        } else if (button == 65) {
            event->scroll_down = 1;
            event->target_x = (x / 2) * 2;
            event->target_y = y;
            return;
        }

        // Ignore other high button numbers (motion events, etc.)
        if (button >= 66) return;

        int btn = button & 0x03;

        if ((button & 0x03) == 0) {
            if (event_type == 'M' && !event->left_just_pressed) {
                event->left_click = 1;
                event->left_just_pressed = true;
                event->target_x = (x / 2) * 2;
                event->target_y = y;
            } else if (event_type == 'm') {
                event->left_just_pressed = false;
            }
        } else if (btn == 2) {
            if (event_type == 'M' && !event->right_just_pressed) {
                event->right_click = 1;
                event->right_just_pressed = true;
                event->target_x = (x / 2) * 2;
                event->target_y = y;
            } else if (event_type == 'm') {
                event->right_just_pressed = false;
            }
        }
    }
}

/**
 * @brief Checks if the given buffer contains a mouse event.
 *
 * @param buffer The buffer to check.
 * @param length The length of the buffer.
 * @return true if the buffer contains a mouse event, false otherwise.
 */
bool is_mouse_event(const char* buffer, size_t length) {
    return length >= 3 && buffer[0] == '\033' && buffer[1] == '[' && buffer[2] == '<';
}

/**
 * @brief Checks if the given buffer contains an arrow key event.
 *
 * @param buffer The buffer to check.
 * @param length The length of the buffer.
 * @return true if the buffer contains an arrow key event, false otherwise.
 */
bool is_arrow_key(const char* buffer, size_t length) {
    return length >= 3 && buffer[0] == '\033' && buffer[1] == '[' && (buffer[2] == 'A' || buffer[2] == 'B' || buffer[2] == 'C' || buffer[2] == 'D');
}

/**
 * @brief Checks if the given buffer contains a bracketed paste start sequence.
 *
 * @param buffer The buffer to check.
 * @param length The length of the buffer.
 * @return true if the buffer contains "\033[200~", false otherwise.
 */
bool is_paste_start(const char* buffer, size_t length) {
    return length >= 6 && buffer[0] == '\033' && buffer[1] == '[' &&
           buffer[2] == '2' && buffer[3] == '0' && buffer[4] == '0' && buffer[5] == '~';
}

/**
 * @brief Checks if the given buffer contains a bracketed paste end sequence.
 *
 * @param buffer The buffer to check.
 * @param length The length of the buffer.
 * @return true if the buffer contains "\033[201~", false otherwise.
 */
bool is_paste_end(const char* buffer, size_t length) {
    return length >= 6 && buffer[0] == '\033' && buffer[1] == '[' &&
           buffer[2] == '2' && buffer[3] == '0' && buffer[4] == '1' && buffer[5] == '~';
}

/**
 * @brief Gets the length of a mouse event in the given buffer.
 *
 * @param buffer The buffer containing the mouse event data.
 * @param length The length of the buffer.
 * @return The length of the mouse event, or 0 if the event is incomplete.
 */
size_t get_mouse_event_length(const char* buffer, size_t length) {
    size_t i = 3;  // Start after "\033[<"

    while (i < length && (isdigit((int)buffer[i]) || buffer[i] == ';')) {
        i++;
    }

    if (i < length && (buffer[i] == 'M' || buffer[i] == 'm')) {
        return i + 1;  // Include the final 'M' or 'm'
    }

    return 0;  // Incomplete mouse event
}

bool get_key_state(unsigned char key) {
    return key_pressed_last_frame[key];
}

void release_key(unsigned char key) {
    key_pressed_last_frame[key] = false;
}

void lock_inputs() {
    unlock = false;
}

void unlock_inputs() {
    unlock = true;
}

void process_input(player** p, Render_Buffer* screen,
                   void (*mouse_left_event_callback)(Render_Buffer* screen, player* p, int x, int y),
                   void (*mouse_right_event_callback)(Render_Buffer* screen, player* p, int x, int y),
                   void (*mouse_scroll_callback)(Render_Buffer* screen, player* p, int x, int y, int direction),
                   void (*printable_char_callback)(Render_Buffer* screen, player* p, int c)) {
    char buffer[128];
    char input_buffer[MAX_BUFFER_SIZE];

    size_t input_buffer_length = 0;
    bool in_paste_mode = false;  // Track if we're inside a paste sequence

    MouseEvent event = {0, 1, false, false, 0, 0, 0, 0};

    while (is_game_running() || unlock == false) {
        ssize_t bytes_read = read_stdin_nonblocking(buffer, sizeof(buffer));

        if (bytes_read <= 0) {
            sys_sleep_ms(1);  // Prevent CPU saturation
            continue;
        }

        // Append new data to the input buffer
        if (input_buffer_length + bytes_read < MAX_BUFFER_SIZE) {
            memcpy(input_buffer + input_buffer_length, buffer, bytes_read);

            input_buffer_length += bytes_read;
        } else {
            LOG_ERROR("Input buffer overflow\n");

            input_buffer_length = 0;  // Clear the buffer in case of overflow

            continue;
        }

        // Process the buffer for complete sequences
        size_t processed = 0;

        while (processed < input_buffer_length) {
            // Check for paste start/end sequences first
            if (is_paste_start(input_buffer + processed, input_buffer_length - processed)) {
                in_paste_mode = true;
                processed += 6;  // Skip "\033[200~"
                continue;
            } else if (is_paste_end(input_buffer + processed, input_buffer_length - processed)) {
                in_paste_mode = false;
                processed += 6;  // Skip "\033[201~"
                continue;
            }

            // If we're in paste mode, skip all characters until we find the end sequence
            if (in_paste_mode) {
                processed++;
                continue;
            }

            if (is_mouse_event(input_buffer + processed, input_buffer_length - processed)) {
                size_t mouse_event_length = get_mouse_event_length(input_buffer + processed, input_buffer_length - processed);
                if (mouse_event_length > 0) {
                    parse_sgr_mouse_event(input_buffer + processed, &event);

                    if (unlock) {
                        player* current_player = *p;
                        if (event.left_click) {
                            mouse_left_event_callback(screen, current_player, event.target_x, event.target_y);
                        }
                        if (event.right_click) {
                            mouse_right_event_callback(screen, current_player, event.target_x, event.target_y);
                        }
                        if (event.scroll_up) {
                            mouse_scroll_callback(screen, current_player, event.target_x, event.target_y, 1);
                        }
                        if (event.scroll_down) {
                            mouse_scroll_callback(screen, current_player, event.target_x, event.target_y, -1);
                        }
                    }

                    processed += mouse_event_length;  // Skip the mouse event
                } else {
                    break;  // Wait for more data
                }
            } else if (is_arrow_key(input_buffer + processed, input_buffer_length - processed)) {
                processed += 3;
            } else if (input_buffer[processed] == CTRL_C) {
                // CTRL+C detected in raw mode
                stop_game();
                processed++;
            } else if ((input_buffer[processed] >= 32 && input_buffer[processed] <= 126) ||
                       input_buffer[processed] == '\n' || input_buffer[processed] == '\r') {
                unsigned char key = (unsigned char)input_buffer[processed];

                if (unlock) {
                    player* current_player = *p;
                    printable_char_callback(screen, current_player, key);
                }

                key_states[key] = true;

                processed++;
            } else {
                // Skip unrecognized or invalid input
                processed++;
            }
        }

        // Store the current state before resetting key_states
        memcpy(key_pressed_last_frame, key_states, sizeof(key_states));

        // Reset key states for next loop iteration
        memset(key_states, 0, sizeof(key_states));

        // Move unprocessed data to the beginning of the buffer
        if (processed < input_buffer_length) {
            memmove(input_buffer, input_buffer + processed, input_buffer_length - processed);
        }

        input_buffer_length -= processed;
    }
}