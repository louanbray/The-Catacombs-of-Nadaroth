#include "render.h"

#include <locale.h>

#include "dynarray.h"
#include "inventory.h"
#include "item.h"
#include "map.h"
#include "player.h"

#define clear_screen() wprintf(L"\033[H\033[J")

/// @brief Render Constants
const int RENDER_WIDTH = 130;
const int RENDER_HEIGHT = 40;

int abs(int x) { return x > 0 ? x : -x; }

board new_screen() {
    board b = malloc(sizeof(wchar_t*) * RENDER_HEIGHT);
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        wchar_t* row = malloc(sizeof(wchar_t) * RENDER_WIDTH);
        b[i] = row;
    }
    white_screen(b);
    return b;
}

void white_screen(board b) {
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        for (int j = 0; j < RENDER_WIDTH; j++) {
            if (i > 0 && (j == 0 || j == RENDER_WIDTH - 1)) {
                if (j == 0) {
                    if (i == 3) {
                        b[i][j] = 9568;
                    } else if (i == RENDER_HEIGHT - 1) {
                        b[i][j] = 9556;
                    } else {
                        b[i][j] = 9553;
                    }
                } else if (j == RENDER_WIDTH - 1) {
                    if (i == 3) {
                        b[i][j] = 9571;
                    } else if (i == RENDER_HEIGHT - 1) {
                        b[i][j] = 9559;
                    } else {
                        b[i][j] = 9553;
                    }
                }
            } else if ((i == 0 || i == RENDER_HEIGHT - 1) || i == 3) {
                if (j != 0 && j != RENDER_WIDTH - 1) {
                    b[i][j] = 9552;
                } else if (i == 0) {
                    if (j == 0) {
                        b[i][j] = 9562;
                    } else {
                        b[i][j] = 9565;
                    }
                }
            } else if (i == 2 && (abs(RENDER_WIDTH / 2 - j) < 10 && j % 2 == 0)) {
                b[i][j] = 9145;
            } else {
                b[i][j] = ' ';
            }
        }
    }
}

/// @brief Place char c on the board with center based coordinates
/// @param b board
/// @param x centered x pos
/// @param y centered y pos
/// @param c char to display
void render_char(board b, int x, int y, int c) {
    b[y + RENDER_HEIGHT / 2][x + RENDER_WIDTH / 2] = c;
}

void render_elements(board b, dynarray* d) {
}

void render_chunk(board b, chunk* c) {
    // TODO : Print chunk based on his type
    // render_char(b, -11, 1, 'x');
    // render_char(b, -11, 2, 'y');
    // render_char(b, -10, 1, get_chunk_x(c) + 48);
    // render_char(b, -10, 2, get_chunk_y(c) + 48);
    // render_elements(b, get_chunk_furniture_list(c));
}

void render_player(board b, player* p) {
    render_char(b, get_player_px(p), get_player_py(p), ' ');
    render_char(b, get_player_x(p), get_player_y(p), get_player_design(p));
}

void render_hotbar(board b, hotbar* h) {
    int display = 57;
    for (int i = 0; i < get_hotbar_max_size(h); i++) {
        if (get_hotbar(h, i) == NULL) {
            b[2][display] = ' ';
        } else {
            b[2][display] = get_item_display(get_hotbar(h, i));
        }
        if (get_selected_slot(h) == i) {
            b[1][display] = 9651;
        } else {
            b[1][display] = ' ';
        }
        display += 2;
    }
}

void render(board b, map* m) {
    player* p = get_player(m);
    chunk* curr = get_player_chunk(p);
    render_chunk(b, curr);
    render_player(b, p);
    render_hotbar(b, get_player_hotbar(p));
}

void update_screen(board b) {
    clear_screen();
    for (int i = RENDER_HEIGHT - 1; i >= 0; i--) {
        wchar_t buffer[RENDER_WIDTH + 2];
        size_t s = 131;
        swprintf(&buffer[0], s, &b[i][0]);
        wprintf(L"%ls\n", buffer);
    }
}