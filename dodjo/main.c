#include <errno.h>
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

/// @brief Get input char, return immediatly
/// @return return char typed
int getch() {
    int acquisition_time = 50;
    int ch;
    struct termios oldattr, newattr;

    if (tcgetattr(STDIN_FILENO, &oldattr) != 0) {
        perror("tcgetattr");
    }

    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    newattr.c_cc[VMIN] = 1;
    newattr.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newattr) != 0) {
        perror("tcsetattr");
    }
    struct pollfd mypoll = {STDIN_FILENO, POLLIN | POLLPRI, POLLOUT};

    if (poll(&mypoll, 1, acquisition_time) > 0) {
        ch = getchar();
    } else if (errno != 0) {
        perror("poll");
    }

    tcflush(STDIN_FILENO, TCIFLUSH);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &oldattr) != 0) {
        perror("tcsetattr");
    }
    return ch;
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

/// @brief Handle the user keyboard entries
/// @param n entry
/// @param b board
/// @param p player
void compute_entry(int n, board b, player* p) {
    if (n == 122 || n == 90 /*Z-z*/) {
        move(b, p, NORTH);
    } else if (n == 115 || n == 83 /*S-s*/) {
        move(b, p, SOUTH);
    } else if (n == 113 || n == 81 /*Q-q*/) {
        move(b, p, WEST);
    } else if (n == 100 || n == 68 /*D-d*/) {
        move(b, p, EAST);
    } else if (n >= 49 && n <= 57 /*1-9*/) {
        select_slot(get_player_hotbar(p), n - 49);
        render_hotbar(b, get_player_hotbar(p));
    } else if (n == 119 || n == 87 /*W-w*/) {
        drop(get_player_hotbar(p), get_selected_slot(get_player_hotbar(p)));
        render_hotbar(b, get_player_hotbar(p));
    } else {
        return;
    }

    update_screen(b);
}

/// @brief Where it all begins
/// @return I dream of a 0
int main() {
    srand(time(NULL));
    setlocale(LC_CTYPE, "");
    board b = new_screen();
    map* m = create_map();

    player* p = create_player(m);
    hotbar* h = create_hotbar();

    link_hotbar(p, h);

    render(b, m);
    update_screen(b);

    item* i = create_item(0, 0, 0, 9733);
    pickup(h, i);

    int n = 0;
    while (true) {  //? Can't put [ESC] char as it is also triggered by the arrows and I don't want the program to close because of an habit. TODO: other char
        n = getch();
        if (n != 0 && n != 7) {
            printf("%d\n", n);  //! REQUIRED BUT WEIRD
            compute_entry(n, b, p);
        }
    }
}