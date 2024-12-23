#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <poll.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

#include "inventory.h"
#include "item.h"
#include "map.h"
#include "player.h"
#include "render.h"

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
void move(board b, player* p, int dir) {
    switch (move_player(p, dir)) {
        case 1:
            render_from_player(b, p);
            break;
        case 2:
            break;
        default:
            render_player(b, p);
            break;
    }
}

int arrow_move(board b, player* p, int c) {
    int k = 10000;
    switch (c) {
        case KEY_ARROW_UP:
            move(b, p, NORTH);
            k = 50000;
            break;
        case KEY_ARROW_DOWN:
            move(b, p, SOUTH);
            k = 50000;
            break;
        case KEY_ARROW_RIGHT:
            move(b, p, EAST);
            break;
        case KEY_ARROW_LEFT:
            move(b, p, WEST);
            break;
    }
    update_screen(b);
    return k;
}

/// @brief Handle the user keyboard entries
/// @param n entry
/// @param b board
/// @param p player
int compute_entry(int n, board b, player* p) {
    int k = 10000;
    if (n == KEY_Z_LOW || n == KEY_Z_HIGH) {
        move(b, p, NORTH);
        k = 50000;
    } else if (n == KEY_S_LOW || n == KEY_S_HIGH) {
        move(b, p, SOUTH);
        k = 50000;
    } else if (n == KEY_Q_LOW || n == KEY_Q_HIGH) {
        move(b, p, WEST);
    } else if (n == KEY_D_LOW || n == KEY_D_HIGH) {
        move(b, p, EAST);
    } else if (n >= 49 && n <= 57 /*1-9*/) {
        select_slot(get_player_hotbar(p), n - 49);
        render_hotbar(b, get_player_hotbar(p));
    } else if (n == 119 || n == 87 /*W-w*/) {
        drop(get_player_hotbar(p), get_selected_slot(get_player_hotbar(p)));
        render_hotbar(b, get_player_hotbar(p));
    } else {
        return 0;
    }

    update_screen(b);
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

    board b = new_screen();
    map* m = create_map();

    player* p = create_player(m);
    hotbar* h = create_hotbar();

    link_hotbar(p, h);

    render(b, m);
    update_screen(b);

    item* it = create_item(0, 0, 0, 9733);
    pickup(h, it);

    while (1) {
        char buffer[10];
        int n = read(STDIN_FILENO, buffer, sizeof(buffer));
        int w = 10000;
        if (n > 0) {
            for (int i = 0; i < 3; i++) {
                if (i > 1 && buffer[i - 1] == '[' && buffer[i - 2] == '\033') {
                    w = arrow_move(b, p, buffer[i]);
                    break;
                } else if (i == 0)
                    w = compute_entry(buffer[i], b, p);
            }
        }
        usleep(w);
    }
    return EXIT_SUCCESS;
}