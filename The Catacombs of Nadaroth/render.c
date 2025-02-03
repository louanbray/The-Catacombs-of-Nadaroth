#include "render.h"

#include <locale.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

#include "assets_manager.h"
#include "dynarray.h"
#include "input_manager.h"
#include "item.h"
#include "map.h"
#include "player.h"

typedef struct Cell {
    wchar_t ch;
    int color;
} Cell;

typedef Cell** board;

typedef struct Render_Buffer {
    board bd;    // current board
    board pv;    // previous board
    board dump;  // dump board
    int* rc;     // row changed flags
} Render_Buffer;

int abs_val(int x) { return x > 0 ? x : -x; }

//
// NEW HELPER FUNCTIONS
//

// Converts a color code to its corresponding ANSI escape sequence.
const char* ansi_from_color(int color) {
    switch (color) {
        case COLOR_RED:
            return "\033[31m";
        case COLOR_CYAN_BOLD:
            return "\033[36;1m";
        case COLOR_YELLOW:
            return "\033[33m";
        case COLOR_MAGENTA_BOLD:
            return "\033[35;1m";
        case COLOR_DEFAULT:
        default:
            return "\033[0m";
    }
}

// Writes a wide string into board b at position (y,x) for a given length and color.
void write_wstr(board b, int y, int x, const wchar_t* str, int len, int color) {
    int i = 0;
    while (i < len && str[i] != L'\0') {
        if (x + i < RENDER_WIDTH) {
            b[y][x + i].ch = str[i];
            b[y][x + i].color = color;
        }
        i++;
    }

    for (; i < len; i++) {
        if (x + i < RENDER_WIDTH) {
            b[y][x + i].ch = L' ';
            b[y][x + i].color = color;
        }
    }
}

// Writes a narrow (char*) string (after converting it to wide characters).
// Here we assume that the string contains only ASCII characters.
void write_str(board b, int y, int x, const char* str, int len, int color) {
    wchar_t wbuffer[RENDER_WIDTH];
    mbstowcs(wbuffer, str, RENDER_WIDTH);
    write_wstr(b, y, x, wbuffer, len, color);
}

//
// BOARD CREATION AND INITIALIZATION
//

// Create a board object
board create_board() {
    Cell* data = malloc(sizeof(Cell) * RENDER_WIDTH * RENDER_HEIGHT);
    board b = malloc(sizeof(Cell*) * RENDER_HEIGHT);
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        b[i] = &data[i * RENDER_WIDTH];
    }
    return b;
}

// Clears the board by setting each cell to a space and default color.
void blank_screen(board b) {
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        for (int j = 0; j < RENDER_WIDTH; j++) {
            b[i][j].ch = L' ';
            b[i][j].color = COLOR_DEFAULT;
        }
    }
}

// Draws the default screen borders and decorations.
void default_screen(board b) {
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        for (int j = 0; j < RENDER_WIDTH; j++) {
            if (i > 0 && (j == 0 || j == RENDER_WIDTH - 1)) {
                if (j == 0) {
                    if (i == 4)
                        b[i][j].ch = 9568;
                    else if (i == RENDER_HEIGHT - 1)
                        b[i][j].ch = 9556;
                    else
                        b[i][j].ch = 9553;
                } else if (j == RENDER_WIDTH - 1) {
                    if (i == 4)
                        b[i][j].ch = 9571;
                    else if (i == RENDER_HEIGHT - 1)
                        b[i][j].ch = 9559;
                    else
                        b[i][j].ch = 9553;
                }
            } else if ((i == 0 || i == RENDER_HEIGHT - 1) || i == 4) {
                if (j != 0 && j != RENDER_WIDTH - 1) {
                    b[i][j].ch = 9552;
                } else if (i == 0) {
                    if (j == 0)
                        b[i][j].ch = 9562;
                    else
                        b[i][j].ch = 9565;
                }
            } else if (i == 3 && (abs_val((RENDER_WIDTH + 1) / 2 - j) < 10 && j % 2 == 0)) {
                b[i][j].ch = 9145;
            } else {
                b[i][j].ch = L' ';
            }
            b[i][j].color = COLOR_DEFAULT;
        }
    }
    write_wstr(b, 3, 2, L"HEALTH: ", 9, COLOR_DEFAULT);
}

// Create the row changed flag array.
int* create_rc() {
    return calloc(RENDER_HEIGHT, sizeof(int));
}

// Create and initialize a new Render_Buffer.
Render_Buffer* create_screen() {
    Render_Buffer* r = malloc(sizeof(Render_Buffer));

    board b = create_board();
    blank_screen(b);

    board pv = create_board();
    blank_screen(pv);

    board dump = create_board();

    r->bd = b;
    r->pv = pv;
    r->dump = dump;
    r->rc = create_rc();
    return r;
}

// Returns the current board.
board get_board(Render_Buffer* r) {
    return r->bd;
}

//
// RENDERING FUNCTIONS
//

// Places a character (with default color) at a position (using center-based coordinates).
void render_char(board b, int x, int y, int c) {
    int draw_y = y + 2 + RENDER_HEIGHT / 2;
    int draw_x = x + RENDER_WIDTH / 2;
    if (draw_y >= 0 && draw_y < RENDER_HEIGHT && draw_x >= 0 && draw_x < RENDER_WIDTH) {
        b[draw_y][draw_x].ch = (wchar_t)c;
        b[draw_y][draw_x].color = COLOR_DEFAULT;
    }
}

wchar_t render_get_cell_char(Render_Buffer* screen, int y, int x) {
    return screen->bd[y][x].ch;
}

// Renders a string onto the render buffer at the specified coordinates.
void render_string(Render_Buffer* screen, int x, int y, char* s, int len) {
    // Adjust coordinates to be relative to the center.
    int draw_x = x + RENDER_WIDTH / 2;
    int draw_y = y + 2 + RENDER_HEIGHT / 2;
    write_str(screen->bd, draw_y, draw_x, s, len, COLOR_DEFAULT);
}

// Renders a wide string onto the render buffer at the specified coordinates.
void render_unicode_string(Render_Buffer* screen, int x, int y, wchar_t* s, int len) {
    int draw_x = x + RENDER_WIDTH / 2;
    int draw_y = y + 2 + RENDER_HEIGHT / 2;
    write_wstr(screen->bd, draw_y, draw_x, s, len, COLOR_DEFAULT);
}

// Renders the title of an item on the screen by writing it into the board with color.
void render_item_title(Render_Buffer* screen, void* it) {
    for (int i = 0; i < 20; i++) {
        screen->bd[1][54 + i].ch = ' ';
        screen->bd[1][54 + i].color = COLOR_DEFAULT;
    }

    update_screen(screen);

    if (it == NULL) return;

    UsableItemAssetFile* uif = get_usable_item_file(get_item_usable_type((item*)it));
    char* title = uif->title;
    Rarity class = uif->specs.specs[0];

    int color = COLOR_DEFAULT;
    switch (class) {
        case BRONZE:
            color = COLOR_RED;
            break;
        case SILVER:
            color = COLOR_CYAN_BOLD;
            break;
        case GOLD:
            color = COLOR_YELLOW;
            break;
        case NADINO:
            color = COLOR_MAGENTA_BOLD;
            break;
        default:
            color = COLOR_DEFAULT;
            break;
    }

    int len = strlen(title);
    int x = (-len + RENDER_WIDTH) / 2 + 2;

    write_str(screen->bd, 1, x, title, len, color);
}

// Renders the chunk contents (furniture items, etc.) into the board.
void render_chunk(Render_Buffer* r, chunk* c) {
    dynarray* d = get_chunk_furniture_list(c);
    board b = r->bd;

    int len = len_dyn(d);
    for (int i = 0; i < len; i++) {
        item* it = get_dyn(d, i);
        if (it == NULL) continue;
        // Render each item (using its own display character)
        render_char(b, get_item_x(it), get_item_y(it), get_item_display(it));
    }

    // Write chunk coordinates onto the board.
    wchar_t tmp[16];
    swprintf(tmp, 16, L"CHUNK X: %d", get_chunk_x(c));
    write_wstr(b, 39, 2, tmp, 16, COLOR_DEFAULT);
    swprintf(tmp, 16, L"CHUNK Y: %d", get_chunk_y(c));
    write_wstr(b, 38, 2, tmp, 16, COLOR_DEFAULT);
}

// Renders the player onto the board.
void render_player(Render_Buffer* r, player* p) {
    render_char(r->bd, get_player_px(p), get_player_py(p), ' ');
    render_char(r->bd, get_player_x(p), get_player_y(p), get_player_design(p));
}

// Renders the health bar into the board.
void render_health(Render_Buffer* r, player* p) {
    int display = 10;
    int health = get_player_health(p);
    int max_health = get_player_max_health(p);
    for (int i = 0; i < max_health; i++) {
        if (i < health) {
            r->bd[3][display].ch = 9829;
        } else {
            r->bd[3][display].ch = 9825;
        }
        r->bd[3][display].color = COLOR_DEFAULT;
        display += 2;
    }
}

// Renders the hotbar (including the selected item indicator) into the board.
void render_hotbar(Render_Buffer* r, hotbar* h) {
    int display = 57;
    int ms = get_hotbar_max_size(h);

    render_item_title(r, get_selected_item(h));

    for (int i = 0; i < ms; i++) {
        item* slot_item = get_hotbar(h, i);
        if (slot_item == NULL) {
            r->bd[3][display].ch = L' ';
        } else {
            r->bd[3][display].ch = get_item_display(slot_item);
        }
        r->bd[3][display].color = COLOR_DEFAULT;

        // Draw a marker above the selected slot.
        if (get_selected_slot(h) == i)
            r->bd[2][display].ch = 9651;
        else
            r->bd[2][display].ch = L' ';
        r->bd[2][display].color = COLOR_DEFAULT;

        display += 2;
    }
}

// Overall render function starting from the map.
void render(Render_Buffer* r, map* m) {
    player* p = get_player(m);
    render_from_player(r, p);
}

// Render from the player’s perspective.
void render_from_player(Render_Buffer* r, player* p) {
    chunk* curr = get_player_chunk(p);
    default_screen(r->bd);
    render_chunk(r, curr);
    render_player(r, p);
    render_hotbar(r, get_player_hotbar(p));
    render_health(r, p);
}

//
// UPDATE SCREEN FUNCTIONS
//

// This function outputs only modified parts of the screen.
// It now also takes into account cell color changes.
void update_screen_(Render_Buffer* r) {
    for (int i = RENDER_HEIGHT - 1; i >= 0; i--) {
        if (!r->rc[i]) continue;
        int screen_row = RENDER_HEIGHT - i;
        r->rc[i] = 0;

        int start = -1;
        int current_color = -1;

        // Buffer to accumulate characters to output.
        wchar_t buffer[RENDER_WIDTH + 1];
        buffer[RENDER_WIDTH] = L'\0';

        for (int j = 0; j < RENDER_WIDTH; j++) {
            Cell curr = r->bd[i][j];
            Cell prev = r->pv[i][j];

            if (curr.ch != prev.ch || curr.color != prev.color) {
                if (start == -1) {
                    start = j;
                    current_color = curr.color;
                }

                r->pv[i][j] = curr;
                buffer[j] = curr.ch;
            } else if (start != -1) {
                buffer[j] = L'\0';
                wprintf(L"\033[%d;%dH%s%ls", screen_row, start + 1, ansi_from_color(current_color), &buffer[start]);
                start = -1;
            }
        }
        if (start != -1) {
            buffer[RENDER_WIDTH] = L'\0';
            wprintf(L"\033[%d;%dH%s%ls", screen_row, start + 1, ansi_from_color(current_color), &buffer[start]);
        }
    }
    fflush(stdout);
}

// Marks rows that have changed.
void mark_changed_rows(Render_Buffer* r) {
#pragma omp parallel for
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        int changed = 0;
        for (int j = 0; j < RENDER_WIDTH; j++) {
            if (r->bd[i][j].ch != r->pv[i][j].ch || r->bd[i][j].color != r->pv[i][j].color) {
                changed = 1;
                break;
            }
        }
        r->rc[i] = changed;
    }
}

//
// TEXT AND CINEMATIC FUNCTIONS
//

// Reads a line from a string source into a buffer.
wchar_t* fgetws_from_string(wchar_t* buffer, int size, const char** source) {
    if (!source || !(*source) || **source == '\0') return NULL;
    int i = 0;
    while (i < size - 1 && **source != '\0' && **source != '\n') {
        buffer[i++] = *((*source)++);
    }
    if (**source == '\n') (*source)++;
    buffer[i] = L'\0';
    return i > 0 ? buffer : NULL;
}

// Prepares the render buffer for modal text display.
void setup_render_buffer(Render_Buffer* r) {
    r->dump = r->bd;
    r->bd = r->pv;
    r->pv = create_board();
    lock_inputs();
}

// Finalizes the render buffer.
void finalize_render_buffer(Render_Buffer* r) {
    free(r->bd);
    r->bd = r->dump;
    update_screen(r);
    unlock_inputs();
}

// Processes a text line (replacing characters after '~' with spaces).
void process_text_line(wchar_t* buffer, size_t width) {
    int eol = 0;
    for (size_t j = 0; j < width - 2; j++) {
        if (buffer[j] == L'~') eol = 1;
        if (eol == 1) buffer[j] = L' ';
    }
}

// Reads text from a file into the render buffer.
void read_text_into_render(Render_Buffer* r, FILE* file) {
    wchar_t buffer[RENDER_WIDTH - 1];
    int i = 0;
    while (fgetws(buffer, RENDER_WIDTH - 1, file) != NULL && i < RENDER_HEIGHT - 2) {
        process_text_line(buffer, RENDER_WIDTH);
        write_wstr(r->bd, RENDER_HEIGHT - i - 2, 1, buffer, wcslen(buffer), COLOR_DEFAULT);
        i++;
    }
    for (; i < RENDER_HEIGHT - 2; i++) {
        if (i != 35)
            for (int j = 1; j < RENDER_WIDTH - 1; j++)
                r->bd[RENDER_HEIGHT - i - 2][j].ch = L' ';
    }
}

// Displays an item description using the render buffer.
void display_item_description(Render_Buffer* r, void* it) {
    item* item_ = (item*)it;
    if (!item_) return;

    UsableItem type = get_item_usable_type(item_);
    if (type == NOT_USABLE_ITEM) return;

    UsableItemAssetFile* item_file = get_usable_item_file(type);
    const char* desc = item_file->description;

    setup_render_buffer(r);

    for (int i = 0; i < 6; i++) {
        for (int j = 1; j < RENDER_WIDTH - 1; j++) {
            r->bd[RENDER_HEIGHT - i - 2][j].ch = L' ';
            r->bd[RENDER_HEIGHT - i - 2][j].color = COLOR_DEFAULT;
        }
    }
    wchar_t buffer[RENDER_WIDTH - 1];
    int i = 6;
    while (fgetws_from_string(buffer, RENDER_WIDTH - 1, &desc) != NULL && i < RENDER_HEIGHT - 2) {
        process_text_line(buffer, RENDER_WIDTH);
        write_wstr(r->bd, RENDER_HEIGHT - i - 2, 1, buffer, RENDER_WIDTH - 2, COLOR_DEFAULT);
        i++;
    }
    for (; i < RENDER_HEIGHT - 2; i++) {
        if (i != 35)
            for (int j = 1; j < RENDER_WIDTH - 1; j++) {
                r->bd[RENDER_HEIGHT - i - 2][j].ch = L' ';
                r->bd[RENDER_HEIGHT - i - 2][j].color = COLOR_DEFAULT;
            }
    }
    render_string(r, -13, 16, " [The Eyes of The Wanderer]", 28);

    wchar_t buff[60];
    swprintf(buff, 50, L"  Analysing [%lc ] - %.*s : ", get_item_display(item_), (int)strlen(item_file->title) - 1, item_file->title);
    render_unicode_string(r, -63, 13, buff, 50);

    update_screen(r);
    while (!USE_KEY('e') && !USE_KEY('E'));

    finalize_render_buffer(r);
}

// Displays an interface (read text from a file) using the render buffer.
void display_interface(Render_Buffer* r, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }
    setup_render_buffer(r);
    render_string(r, -8, -18, " PRESS [H] TO EXIT", 19);
    read_text_into_render(r, file);
    update_screen(r);
    while (!USE_KEY('H') && !USE_KEY('h'));
    finalize_render_buffer(r);
    fclose(file);
}

// Plays a cinematic by reading lines from a file and updating the board.
void play_cinematic(Render_Buffer* r, const char* filename, int delay) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    setup_render_buffer(r);
    render_string(r, -10, -18, " PRESS [SPACE] TO SKIP", 23);

    wchar_t buffer[RENDER_WIDTH + 4];
    int row = 0;
    while (fgetws(buffer, RENDER_WIDTH + 4, file) != NULL && row < RENDER_HEIGHT - 2) {
        if (KEY_PRESSED(' ')) break;
        if (buffer[0] == L'#') {
            int timeout = 0;
            for (int j = 0; j < RENDER_WIDTH - 1; j++)
                if (buffer[j] == L'#') timeout++;
            for (; row < RENDER_HEIGHT - 2; row++) {
                if (row != 35)
                    for (int j = 1; j < RENDER_WIDTH - 1; j++) {
                        r->bd[RENDER_HEIGHT - row - 2][j].ch = L' ';
                        r->bd[RENDER_HEIGHT - row - 2][j].color = COLOR_DEFAULT;
                    }
            }
            row = 0;
            update_screen(r);
            usleep(delay * timeout);
        } else if (buffer[0] == L'%') {
            int eol = 0;
            for (int j = 1; j < RENDER_WIDTH - 2; j++) {
                if (buffer[j] == L'~') eol = 1;
                if (eol == 1) buffer[j] = L' ';
                r->bd[RENDER_HEIGHT - row - 2][j].ch = buffer[j];
                r->bd[RENDER_HEIGHT - row - 2][j].color = COLOR_DEFAULT;
                if (!eol) {
                    update_screen(r);
                    usleep(delay / 50);
                }
            }
            update_screen(r);
            row++;
        } else {
            process_text_line(buffer, RENDER_WIDTH);
            write_wstr(r->bd, RENDER_HEIGHT - row - 2, 1, buffer, wcslen(buffer), COLOR_DEFAULT);
            row++;
        }
    }
    finalize_render_buffer(r);
    fclose(file);
}

// Mark rows as changed and update the screen.
void update_screen(Render_Buffer* r) {
    mark_changed_rows(r);
    update_screen_(r);
}
