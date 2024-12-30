#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <poll.h>
#include <stdlib.h>
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
    wprintf(L"\33[?25h");      // Re-enable cursor visibility
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
    t.c_cc[VMIN] = 0;               // Minimum number of characters to read
    t.c_cc[VTIME] = 0;              // Timeout (deciseconds) for read

    // Set the modified attributes
    if (tcsetattr(STDIN_FILENO, TCSANOW, &t) < 0) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
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

/// @brief Where it all begins
/// @return I dream of a 0
int main() {
    setup_terminal_restoration();

    set_raw_mode();
    set_nonblocking_mode(STDIN_FILENO);

    srand(time(NULL));

    setlocale(LC_CTYPE, "");
    wprintf(L"\33[?25l");
    wprintf(L"\033[H\033[J");

    renderbuffer* screen = create_screen();

    map* m = create_map();

    player* p = create_player(m);
    hotbar* h = create_hotbar();

    link_hotbar(p, h);

    render(screen, m);
    update_screen(screen);

    while (1) {
        char buffer[10];
        int n = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (n > 0) {
            for (int i = 0; i < 3; i++) {
                if (i > 1 && buffer[i - 1] == '[' && buffer[i - 2] == '\033') {
                    arrow_move(screen, p, buffer[i]);
                    break;
                } else if (i == 0)
                    compute_entry(buffer[i], screen, p);
            }
        }
        usleep(10000);
    }
    return EXIT_SUCCESS;
}