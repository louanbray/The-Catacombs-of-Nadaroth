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

int arrow_move(renderbuffer* screen, player* p, int c) {
    int k = 10000;
    switch (c) {
        case KEY_ARROW_UP:
            move(screen, p, NORTH);
            k = 50000;
            break;
        case KEY_ARROW_DOWN:
            move(screen, p, SOUTH);
            k = 50000;
            break;
        case KEY_ARROW_RIGHT:
            move(screen, p, EAST);
            break;
        case KEY_ARROW_LEFT:
            move(screen, p, WEST);
            break;
    }
    update_screen(screen);
    return k;
}

/// @brief Handle the user keyboard entries
/// @param n entry
/// @param b board
/// @param p player
int compute_entry(int n, renderbuffer* screen, player* p) {
    int k = 10000;
    if (n == KEY_Z_LOW || n == KEY_Z_HIGH) {
        move(screen, p, NORTH);
        k = 30000;
    } else if (n == KEY_S_LOW || n == KEY_S_HIGH) {
        move(screen, p, SOUTH);
        k = 30000;
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
        return 0;
    }

    update_screen(screen);
    return k;
}

/// @brief Where it all begins
/// @return I dream of a 0
int main() {
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

    // item* it = create_item(0, 0, 0, 9733, -1);
    // pickup(h, it);

    while (1) {
        char buffer[10];
        int n = read(STDIN_FILENO, buffer, sizeof(buffer));
        int w = 10000;
        if (n > 0) {
            for (int i = 0; i < 3; i++) {
                if (i > 1 && buffer[i - 1] == '[' && buffer[i - 2] == '\033') {
                    w = arrow_move(screen, p, buffer[i]);
                    break;
                } else if (i == 0)
                    w = compute_entry(buffer[i], screen, p);
            }
        }
        usleep(w);
    }
    return EXIT_SUCCESS;
}