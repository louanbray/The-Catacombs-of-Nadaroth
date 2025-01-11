#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

#include "item.h"
#include "map.h"
#include "player.h"
#include "render.h"

#define CHAR_TO_INT 49

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

/// @brief Handle the player movement and use the appropriate render
/// @param b board
/// @param p player
/// @param dir direction
void move(renderbuffer* screen, player* p, int dir) {
    switch (move_player(p, dir)) {
        case 1:
            render_from_player(screen, p);
            break;
        case 2:
            break;
        case 3:
            render_hotbar(screen, get_player_hotbar(p));
            render_player(screen, p);
            break;
        default:
            render_player(screen, p);
            break;
    }
}

void arrow_move(renderbuffer* screen, player* p, int c) {
    switch (c) {
        case KEY_ARROW_UP:
            move(screen, p, NORTH);
            break;
        case KEY_ARROW_DOWN:
            move(screen, p, SOUTH);
            break;
        case KEY_ARROW_RIGHT:
            move(screen, p, EAST);
            break;
        case KEY_ARROW_LEFT:
            move(screen, p, WEST);
            break;
    }
    update_screen(screen);
}

/// @brief Handle the user keyboard entries
/// @param n entry
/// @param b board
/// @param p player
void compute_entry(int n, renderbuffer* screen, player* p) {
    if (n == KEY_Z_LOW || n == KEY_Z_HIGH) {
        move(screen, p, NORTH);
    } else if (n == KEY_S_LOW || n == KEY_S_HIGH) {
        move(screen, p, SOUTH);
    } else if (n == KEY_Q_LOW || n == KEY_Q_HIGH) {
        move(screen, p, WEST);
    } else if (n == KEY_D_LOW || n == KEY_D_HIGH) {
        move(screen, p, EAST);
    } else if (n <= KEY_9 && n >= KEY_1) {
        select_slot(get_player_hotbar(p), n - CHAR_TO_INT);
        render_hotbar(screen, get_player_hotbar(p));
    } else if (n == KEY_W_HIGH || n == KEY_W_LOW) {
        drop(get_player_hotbar(p), get_selected_slot(get_player_hotbar(p)));
        render_hotbar(screen, get_player_hotbar(p));
    } else {
        return;
    }

    update_screen(screen);
}

/// @brief Setup the terminal for display and inputs
void init() {
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

// // Function to draw a line using Bresenham's Algorithm
// void draw_line(int x0, int y0, int x1, int y1, char symbol) {
//     int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
//     int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
//     int err = dx + dy, e2;

//     while (1) {
//         wprintf(L"\033[%d;%dH%c", y0, x0, symbol);  // Move to (y0, x0) and draw symbol
//         fflush(stdout);

//         if (x0 == x1 && y0 == y1) break;  // Line complete
//         e2 = 2 * err;
//         if (e2 >= dy) {
//             err += dy;
//             x0 += sx;
//         }
//         if (e2 <= dx) {
//             err += dx;
//             y0 += sy;
//         }
//     }
// }

// Function to animate a projectile moving from (x0, y0) to (x1, y1)
void animate_projectile(int x0, int y0, int x1, int y1, player* p, renderbuffer* screen) {
    int x = 0, y = -101;
    // wprintf(L"Mouse Event: Button Code = %d, X = %d, Y = %d\n", 1, x1, y1);
    render_projectile(x0, y0, x1, y1, &x, &y, screen);

    if (y != -101) {
        item* it = get_hm(get_chunk_furniture_coords(get_player_chunk(p)), x, y);
        if (it == NULL) return;
        // wprintf(L"Target hit ! (%lc)", get_item_display(it));
        fflush(stdout);
    }
    //! OK
}

void parse_sgr_mouse_event(char* buffer, int* target_x, int* target_y, int* left_click) {
    int button, x, y;
    char event_type;

    if (sscanf(buffer, "\033[<%d;%d;%d%c", &button, &x, &y, &event_type) == 4) {
        *left_click = (button & 0x03) == 0 && (event_type == 'M');
        *target_x = x = (x / 2) * 2;
        *target_y = y;
    }
}

/// @brief Where it all begins
/// @return I dream of a 0
int main() {
    init();

    renderbuffer* screen = create_screen();

    map* m = create_map();

    player* p = create_player(m);
    hotbar* h = create_hotbar();

    link_hotbar(p, h);

    render(screen, m);
    update_screen(screen);

    // play_cinematic(screen, "assets/cinematics/example.dodjo", 5000000);

    char buffer[32];
    int target_x = 0, target_y = 1;  // Mouse target position
    int last_x = 0, last_y = 1;
    int left_click = 0;  // Flag for left click
    while (1) {
        memset(buffer, 0, sizeof(buffer));           // Clear the buffer
        read(STDIN_FILENO, buffer, sizeof(buffer));  // Read input

        if (buffer[0] == '\033') {  // Escape sequence detected
            if (buffer[1] == '[' && buffer[2] == '<') {
                wprintf(L"\033[?1003l\033[?1006l");
                parse_sgr_mouse_event(buffer, &target_x, &target_y, &left_click);
                if (left_click && (target_x != last_x || target_y != last_y)) {
                    last_x = target_x;
                    last_y = target_y;
                    animate_projectile(get_player_x(p) + 65, -get_player_y(p) + 19, target_x, target_y, p, screen);
                    left_click = 0;
                }
                wprintf(L"\033[?1003h\033[?1006h");
                fflush(stdout);
                // else {
                //     draw_line(65, 20, target_x, target_y, '.');
                // }
            } else if (buffer[1] == '[') {
                arrow_move(screen, p, buffer[2]);
            }
        } else {
            compute_entry(buffer[0], screen, p);
        }
    }
    return EXIT_SUCCESS;
}