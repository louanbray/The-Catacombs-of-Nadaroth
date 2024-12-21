#include "render.h"

#include "dynarray.h"
#include "inventory.h"
#include "item.h"
#include "map.h"
#include "player.h"

#define clear_screen() printf("\033[H\033[J")

/// @brief Render Constants
const int RENDER_WIDTH = 130;
const int RENDER_HEIGHT = 40;

int abs(int x) { return x > 0 ? x : -x; }

board new_screen() {
    board b = malloc(sizeof(char*) * RENDER_HEIGHT);
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        char* row = malloc(sizeof(char) * RENDER_WIDTH);
        for (int j = 0; j < RENDER_WIDTH; j++) {
            if (i > 2 && (((i == 0 || i == RENDER_HEIGHT - 1) || i == 3) || (j == 0 || j == RENDER_WIDTH - 1))) {
                row[j] = '.';
            } else if (i == 2 && (abs(RENDER_WIDTH / 2 - j) < 10 && j % 2 == 0)) {
                row[j] = '|';
            } else {
                row[j] = ' ';
            }
        }
        b[i] = row;
    }
    return b;
}

void white_screen(board b) {
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        for (int j = 0; j < RENDER_WIDTH; j++) {
            if (i > 2 && (((i == 0 || i == RENDER_HEIGHT - 1) || i == 3) || (j == 0 || j == RENDER_WIDTH - 1))) {
                b[i][j] = '.';
            } else if (i == 2 && (abs(RENDER_WIDTH / 2 - j) < 10 && j % 2 == 0)) {
                b[i][j] = '|';
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
void render_char(board b, int x, int y, char c) {
    b[y + RENDER_HEIGHT / 2][x + RENDER_WIDTH / 2] = c;
}

void render_elements(board b, dynarray* d) {
}

void render_chunk(board b, chunk* c) {
    // TODO : Print chunk based on his type
    render_char(b, -11, 1, 'x');
    render_char(b, -11, 2, 'y');
    render_char(b, -10, 1, get_chunk_x(c) + 48);
    render_char(b, -10, 2, get_chunk_y(c) + 48);
    render_elements(b, get_chunk_furniture_list(c));
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
            b[1][display] = '^';
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
        char buffer[RENDER_WIDTH + 1];
        sprintf(&buffer[0], "%130s", &b[i][0]);
        printf("%130s\n", buffer);
    }
}