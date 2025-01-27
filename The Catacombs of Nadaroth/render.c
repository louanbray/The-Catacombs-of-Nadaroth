#include "render.h"

#include <locale.h>
#include <omp.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "assets_manager.h"
#include "dynarray.h"
#include "item.h"
#include "map.h"
#include "player.h"

typedef struct Render_Buffer {
    board bd;
    board pv;
    int* rc;
} Render_Buffer;

int abs(int x) { return x > 0 ? x : -x; }

/// @brief Create a board object
/// @return board
board create_board() {
    wchar_t* data = malloc(sizeof(wchar_t) * RENDER_WIDTH * RENDER_HEIGHT);
    board b = malloc(sizeof(wchar_t*) * RENDER_HEIGHT);

    for (int i = 0; i < RENDER_HEIGHT; i++) {
        b[i] = &data[i * RENDER_WIDTH];
    }

    return b;
}

void blank_screen(board b) {
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        for (int j = 0; j < RENDER_WIDTH; j++) {
            b[i][j] = ' ';
        }
    }
}
void default_screen(board b) {
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        for (int j = 0; j < RENDER_WIDTH; j++) {
            if (i > 0 && (j == 0 || j == RENDER_WIDTH - 1)) {
                if (j == 0) {
                    if (i == 4) {
                        b[i][j] = 9568;
                    } else if (i == RENDER_HEIGHT - 1) {
                        b[i][j] = 9556;
                    } else {
                        b[i][j] = 9553;
                    }
                } else if (j == RENDER_WIDTH - 1) {
                    if (i == 4) {
                        b[i][j] = 9571;
                    } else if (i == RENDER_HEIGHT - 1) {
                        b[i][j] = 9559;
                    } else {
                        b[i][j] = 9553;
                    }
                }
            } else if ((i == 0 || i == RENDER_HEIGHT - 1) || i == 4) {
                if (j != 0 && j != RENDER_WIDTH - 1) {
                    b[i][j] = 9552;
                } else if (i == 0) {
                    if (j == 0) {
                        b[i][j] = 9562;
                    } else {
                        b[i][j] = 9565;
                    }
                }
            } else if (i == 3 && (abs((RENDER_WIDTH + 1) / 2 - j) < 10 && j % 2 == 0)) {
                b[i][j] = 9145;
            } else {
                b[i][j] = ' ';
            }
        }
    }
    swprintf(&b[3][2], 9, L"HEALTH: ");
}

/// @brief Create the row changed flag array
/// @return row changed array
int* create_rc() {
    return calloc(RENDER_HEIGHT, sizeof(int));
}

Render_Buffer* create_screen() {
    Render_Buffer* r = malloc(sizeof(Render_Buffer));

    board b = create_board();
    blank_screen(b);

    board pv = create_board();
    blank_screen(pv);

    r->bd = b;
    r->pv = pv;
    r->rc = create_rc();
    return r;
}

board get_board(Render_Buffer* r) {
    return r->bd;
}

/// @brief Place char c on the board with center based coordinates
/// @param b board
/// @param x centered x pos
/// @param y centered y pos
/// @param c char to display
void render_char(board b, int x, int y, int c) {
    b[y + 2 + RENDER_HEIGHT / 2][x + RENDER_WIDTH / 2] = c;
}

void render_string(Render_Buffer* screen, int x, int y, char* s, int len) {
    x += RENDER_WIDTH / 2;
    y += 2 + RENDER_HEIGHT / 2;

    swprintf(&screen->bd[y][x], len, L"%s", s);
    for (int i = wcslen(&screen->bd[y][x]); i < len; i++) screen->bd[y][x + i] = L' ';
}

void render_item_title(void* it) {
    wprintf(L"\033[%d;%dH                    ", 40, 55);
    if (it == NULL) return;

    UsableItemAssetFile* uif = get_usable_item_file(get_item_usable_type((item*)it));
    char* title = uif->title;
    Rarity class = uif->specs.specs[0];

    char buffer[10];

    switch (class) {
        case BRONZE:
            sprintf(buffer, "\033[31m");
            break;
        case SILVER:
            sprintf(buffer, "\033[36;1m");
            break;
        case GOLD:
            sprintf(buffer, "\033[33m");
            break;
        case NADINO:
            sprintf(buffer, "\033[35;1m");
            break;
    }

    int len = strlen(title);

    int x = (-len + RENDER_WIDTH) / 2 + 3;

    wprintf(L"\033[%d;%dH%s%s\033[0m", 40, x, buffer, title);
}

void render_chunk(Render_Buffer* r, chunk* c) {
    dynarray* d = get_chunk_furniture_list(c);
    board b = r->bd;

    int len = len_dyn(d);

    for (int i = 0; i < len; i++) {
        item* it = get_dyn(d, i);

        if (it == NULL) continue;

        render_char(b, get_item_x(it), get_item_y(it), get_item_display(it));
    }

    swprintf(&b[39][2], 16, L"CHUNK X: %d", get_chunk_x(c));
    for (int i = wcslen(&b[39][2]); i < 16; i++) b[39][2 + i] = L' ';
    swprintf(&b[38][2], 16, L"CHUNK Y: %d", get_chunk_y(c));
    for (int i = wcslen(&b[38][2]); i < 16; i++) b[38][2 + i] = L' ';
}

void render_player(Render_Buffer* r, player* p) {
    render_char(r->bd, get_player_px(p), get_player_py(p), ' ');
    render_char(r->bd, get_player_x(p), get_player_y(p), get_player_design(p));
}

void render_health(Render_Buffer* r, player* p) {
    int display = 10;
    int health = get_player_health(p);
    int max_health = get_player_max_health(p);

    for (int i = 0; i < max_health; i++) {
        if (i < health) {
            r->bd[3][display] = 9829;
        } else {
            r->bd[3][display] = 9825;
        }

        display += 2;
    }
}

void render_hotbar(Render_Buffer* r, hotbar* h) {
    int display = 57;
    int ms = get_hotbar_max_size(h);

    render_item_title(get_selected_item(h));

    for (int i = 0; i < ms; i++) {
        if (get_hotbar(h, i) == NULL) {
            r->bd[3][display] = ' ';
        } else {
            r->bd[3][display] = get_item_display(get_hotbar(h, i));
        }

        if (get_selected_slot(h) == i) {
            r->bd[2][display] = 9651;
        } else {
            r->bd[2][display] = ' ';
        }

        display += 2;
    }
}

void render(Render_Buffer* r, map* m) {
    player* p = get_player(m);
    render_from_player(r, p);
}

void render_from_player(Render_Buffer* r, player* p) {
    chunk* curr = get_player_chunk(p);

    default_screen(r->bd);
    render_chunk(r, curr);

    render_player(r, p);
    render_hotbar(r, get_player_hotbar(p));
    render_health(r, p);
}

/// @brief Print only modified parts of the screen based on the buffer
/// @param r render buffer
void update_screen_(Render_Buffer* r) {
    for (int i = RENDER_HEIGHT - 1; i >= 0; i--) {
        if (!r->rc[i]) continue;

        int screen_row = RENDER_HEIGHT - i;

        r->rc[i] = 0;

        wchar_t buffer[RENDER_WIDTH + 1];
        buffer[RENDER_WIDTH] = L'\0';

        int start = -1;

        for (int j = 0; j < RENDER_WIDTH; j++) {
            if (r->bd[i][j] != r->pv[i][j]) {
                if (start == -1) start = j;

                buffer[j] = r->bd[i][j];
                r->pv[i][j] = r->bd[i][j];
            } else if (start != -1) {
                buffer[j] = L'\0';

                wprintf(L"\033[%d;%dH%ls", screen_row, start + 1, &buffer[start]);

                start = -1;
            }
        }
        if (start != -1) {
            buffer[RENDER_WIDTH] = L'\0';
            wprintf(L"\033[%d;%dH%ls", screen_row, start + 1, &buffer[start]);
        }
    }
    fflush(stdout);
}

/// @brief Update changed rows between buffer arrays
/// @param r render buffer
void mark_changed_rows(Render_Buffer* r) {
#pragma omp parallel for
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        int changed = 0;

        for (int j = 0; j < RENDER_WIDTH; j++) {
            if (r->bd[i][j] != r->pv[i][j]) {
                changed = 1;
                break;
            }
        }

        r->rc[i] = changed;
    }
}

void display_interface(Render_Buffer* r, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    wchar_t buffer[RENDER_WIDTH - 1];
    int i = 0;

    while (fgetws(buffer, RENDER_WIDTH - 1, file) != NULL && i < RENDER_HEIGHT - 2) {
        int a = 0;  //? TO REMOVE THE EOL AFTER ~

        for (int j = 0; j < RENDER_WIDTH - 2; j++) {
            if (buffer[j] == '~') a = 1;
            if (a == 1) buffer[j] = L' ';
        }

        swprintf(&r->bd[RENDER_HEIGHT - i - 2][1], RENDER_WIDTH - 1, L"%ls", buffer);
        i++;
    }

    for (; i < RENDER_HEIGHT - 2; i++) {
        for (int j = 0; j < RENDER_WIDTH - 1; j++) {
            if (i != 35) {
                buffer[j] = ' ';
                wcsncpy(&r->bd[RENDER_HEIGHT - i - 2][1], buffer, RENDER_WIDTH - 2);
            }
        }
    }

    fclose(file);
    update_screen(r);
}

void play_cinematic(Render_Buffer* r, const char* filename, int delay) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    wchar_t buffer[180];

    int row = 0;

    //* +4 pour gérer les \r \t \0 \n
    while (fgetws(buffer, RENDER_WIDTH + 4, file) != NULL && row < RENDER_HEIGHT - 2) {
        if (buffer[0] == '#') {
            int timeout = 0;

            for (; row < RENDER_HEIGHT - 2; row++) {
                for (int j = 0; j < RENDER_WIDTH - 1; j++) {
                    if (buffer[j] == '#') timeout++;
                    buffer[j] = ' ';
                }

                if (row != 35) wcsncpy(&r->bd[RENDER_HEIGHT - row - 2][1], buffer, RENDER_WIDTH - 2);
            }

            row = 0;
            update_screen(r);

            usleep(delay * timeout);
        } else if (buffer[0] == '%') {
            int eol = 0;

            for (int j = 1; j < RENDER_WIDTH - 2; j++) {
                if (buffer[j] == '~') eol = 1;
                if (eol == 1) buffer[j] = L' ';
                r->bd[RENDER_HEIGHT - row - 2][j] = buffer[j];
                if (!eol) {
                    update_screen(r);
                    usleep(delay / 50);
                }
            }

            update_screen(r);
            row++;
        } else {
            int eol = 0;

            for (int j = 0; j < RENDER_WIDTH - 2; j++) {
                if (buffer[j] == '~') eol = 1;
                if (eol == 1) buffer[j] = L' ';
            }

            swprintf(&r->bd[RENDER_HEIGHT - row - 2][1], RENDER_WIDTH - 1, L"%ls", buffer);
            row++;
        }
    }

    fclose(file);
}

void update_screen(Render_Buffer* r) {
    mark_changed_rows(r);
    update_screen_(r);
}