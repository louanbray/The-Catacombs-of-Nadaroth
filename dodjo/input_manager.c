#include "input_manager.h"

#define MAX_BUFFER_SIZE 1024

static struct termios original_termios;  // Global to store original terminal settings

/// @brief Save the current terminal settings
void save_original_mode() {
    if (tcgetattr(STDIN_FILENO, &original_termios) < 0) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
}

/// @brief Restore the terminal to its original mode
void restore_terminal_mode() {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &original_termios) < 0) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
    wprintf(L"\33[?25h");  // Re-enable cursor visibility
    wprintf(L"\033[?1003l\033[?1006l");
    wprintf(L"\033[H\033[J");  // Clear the screen
    fflush(stdout);
}

/// @brief Signal handler to restore terminal on unexpected termination
void handle_signal(int signo) {
    restore_terminal_mode();
    exit(EXIT_FAILURE);
}

/// @brief Ensure restoration on program exit
void setup_terminal_restoration() {
    save_original_mode();
    atexit(restore_terminal_mode);   // Ensure restoration on normal exit
    signal(SIGINT, handle_signal);   // Handle Ctrl+C (SIGINT)
    signal(SIGTERM, handle_signal);  // Handle termination signals
}

// Function to set the terminal to raw mode
void set_raw_mode() {
    struct termios t;

    // Get the current terminal attributes
    if (tcgetattr(STDIN_FILENO, &t) < 0) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Modify the terminal attributes for raw mode
    t.c_lflag &= ~(ICANON | ECHO);  // Disable canonical mode and echo
    t.c_cc[VMIN] = 1;               // Minimum number of characters to read
    t.c_cc[VTIME] = 0;              // Timeout (deciseconds) for read

    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// Function to set the file descriptor to non-blocking mode
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

void init_terminal() {
    setup_terminal_restoration();

    set_raw_mode();
    set_nonblocking_mode(STDIN_FILENO);

    srand(time(NULL));

    setlocale(LC_CTYPE, "");
    wprintf(L"\33[?25l");                // Disable cursor
    wprintf(L"\033[H\033[J");            // Clear
    wprintf(L"\033[?1003h\033[?1006h");  // Enable mouse motion events
    fflush(stdout);
}

void parse_sgr_mouse_event(const char* buffer, int* target_x, int* target_y, int* left_click, bool* just_pressed) {
    int button, x, y;
    char event_type;

    *left_click = 0;
    if (sscanf(buffer, "\033[<%d;%d;%d%c", &button, &x, &y, &event_type) == 4) {
        if (button >= 64) return;

        if ((button & 0x03) == 0) {
            if (event_type == 'M' && !(*just_pressed)) {
                *left_click = 1;
                *just_pressed = true;
                *target_x = (x / 2) * 2;
                *target_y = y;
            } else if (event_type == 'm') {
                *just_pressed = false;
            }
        }
    }
}

bool is_mouse_event(const char* buffer, size_t length) {
    return length >= 3 && buffer[0] == '\033' && buffer[1] == '[' && buffer[2] == '<';
}

bool is_arrow_key(const char* buffer, size_t length) {
    return length >= 3 && buffer[0] == '\033' && buffer[1] == '[' && (buffer[2] == 'A' || buffer[2] == 'B' || buffer[2] == 'C' || buffer[2] == 'D');
}

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

void process_input(player* p, Render_Buffer* screen,
                   void (*mouse_event_callback)(Render_Buffer* screen, player* p, int x, int y),
                   void (*arrow_key_callback)(Render_Buffer* screen, player* p, int arrow_key),
                   void (*printable_char_callback)(Render_Buffer* screen, player* p, int c)) {
    char buffer[128];
    char input_buffer[MAX_BUFFER_SIZE];

    size_t input_buffer_length = 0;

    int target_x = 0, target_y = 1;  // Mouse target position
    int left_click = 0;              // Flag for left click
    bool just_pressed = false;

    while (1) {
        ssize_t bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer));

        if (bytes_read <= 0) {
            usleep(1000);  // Prevent CPU saturation
            continue;
        }

        // Append new data to the input buffer
        if (input_buffer_length + bytes_read < MAX_BUFFER_SIZE) {
            memcpy(input_buffer + input_buffer_length, buffer, bytes_read);

            input_buffer_length += bytes_read;
        } else {
            fprintf(stderr, "Input buffer overflow\n");

            input_buffer_length = 0;  // Clear the buffer in case of overflow

            continue;
        }

        // Process the buffer for complete sequences
        size_t processed = 0;

        while (processed < input_buffer_length) {
            if (is_mouse_event(input_buffer + processed, input_buffer_length - processed)) {
                size_t mouse_event_length = get_mouse_event_length(input_buffer + processed, input_buffer_length - processed);
                if (mouse_event_length > 0) {
                    parse_sgr_mouse_event(input_buffer + processed, &target_x, &target_y, &left_click, &just_pressed);

                    if (left_click) {
                        mouse_event_callback(screen, p, target_x, target_y);
                    }

                    processed += mouse_event_length;  // Skip the mouse event
                } else {
                    break;  // Wait for more data
                }
            } else if (is_arrow_key(input_buffer + processed, input_buffer_length - processed)) {
                arrow_key_callback(screen, p, (input_buffer + processed)[2]);

                processed += 3;  // Arrow keys are 3 bytes
            } else if ((input_buffer[processed] >= 32 && input_buffer[processed] <= 126) ||
                       input_buffer[processed] == '\n' || input_buffer[processed] == '\r') {
                printable_char_callback(screen, p, input_buffer[processed]);

                processed++;
            } else {
                // Skip unrecognized or invalid input
                processed++;
            }
        }

        // Shift unprocessed data to the beginning of the buffer
        if (processed < input_buffer_length) {
            memmove(input_buffer, input_buffer + processed, input_buffer_length - processed);
        }

        input_buffer_length -= processed;
    }
}