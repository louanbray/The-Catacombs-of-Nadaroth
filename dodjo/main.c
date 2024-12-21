#include <errno.h>
#include <locale.h>
#include <poll.h>
#include <stdlib.h>
#include <termios.h>
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

void compute_entry(int n, board b, player* p) {
    if (n == 122 || n == 90 /*Z-z*/) {
        move_player(p, NORTH);
        // move_player_chunk(p, NORTH);
    } else if (n == 115 || n == 83 /*S-s*/) {
        move_player(p, SOUTH);
        // move_player_chunk(p, SOUTH);
    } else if (n == 113 || n == 81 /*Q-q*/) {
        move_player(p, WEST);
        // move_player_chunk(p, WEST);
    } else if (n == 100 || n == 68 /*D-d*/) {
        move_player(p, EAST);
        // move_player_chunk(p, EAST);
    } else if (n == 110 || n == 78 /*N-n*/) {
        move_player_chunk(p, NORTH);
        render_from_player(b, p);
    } else if (n == 98 || n == 66 /*B-b*/) {
        move_player_chunk(p, SOUTH);
        render_from_player(b, p);
    } else if (n >= 49 && n <= 57) {
        select_slot(get_player_hotbar(p), n - 49);
        render_hotbar(b, get_player_hotbar(p));
    } else if (n == 119 || n == 87 /*W-w*/) {
        drop(get_player_hotbar(p), get_selected_slot(get_player_hotbar(p)));
        render_hotbar(b, get_player_hotbar(p));
    } else {
        return;
    }
    // render_chunk(b, get_player_chunk(p));
    render_player(b, p);
    update_screen(b);
}

int main() {
    setlocale(LC_CTYPE, "");
    board b = new_screen();
    map* m = create_map();

    player* p = create_player(m);
    hotbar* h = create_hotbar();

    link_hotbar(p, h);

    render(b, m);
    update_screen(b);

    item* i = create_item(0, 0, 0);
    pickup(h, i);

    int n = 0;
    while (n != 27) {
        n = getch();
        if (n != 0 && n != 7) {
            printf("");  //! REQUIRED BUT WEIRD
            compute_entry(n, b, p);
        }
    }
}