#include "render.h"

#include <omp.h>
#include <time.h>

#include "achievements.h"
#include "assets_manager.h"
#include "audio_manager.h"
#include "dynarray.h"
#include "game_status.h"
#include "input_manager.h"
#include "item.h"
#include "logger.h"
#include "map.h"
#include "player.h"
#include "projectile.h"
#include "save_manager.h"
#include "settings.h"
#include "statistics.h"

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

#define INFO_ROW_BOTTOM 1
#define INFO_ROW_MID 2
#define INFO_ROW_TOP 3

#define MAX_LINE 512

// For the outline box
#define JUNCTION_HEIGHT 4
#define REVERSED_INBOX_JUNCTION_HEIGHT (RENDER_HEIGHT - JUNCTION_HEIGHT - 2)

#define HOTBAR_START ((RENDER_WIDTH + 1) / 2 - 10)
#define HOTBAR_END ((RENDER_WIDTH + 1) / 2 + 10)

#define TITLE_ERASE_START (RENDER_WIDTH / 2 - 10)
#define MAX_TITLE_SIZE 20

#define CHUNK_DISPLAY_X_POS 115
#define TIMER_DISPLAY_X_POS 110
#define SCORE_DISPLAY_X_POS 9
#define MENTAL_HEALTH_DIPLAY_X_POS 17
#define HOTBAR_DISPLAY_X_POS 57

// Item Description GUI
#define TITLE_DISPLAY_X_POS -13
#define TITLE_DISPLAY_Y_POS 16
#define ANALYSIS_DISPLAY_X_POS -63
#define ANALYSIS_DISPLAY_Y_POS 13

#define SPACE_TO_EXIT_DISPLAY_X_POS -10
#define SPACE_TO_EXIT_DISPLAY_Y_POS -18

#define ITEM_DESCRIPTION_Y_OFFSET 6

// Achievements
#define MAX_ACH_PER_PAGE 10
#define ACH_ENTRY_SPACING 3
#define ACH_X_OFFSET 6
#define ACH_ARROW_PREV_X 2
#define ACH_ARROW_NEXT_X (RENDER_WIDTH - 5)
#define ACH_GUI_TITLE_Y 16

// Settings
#define MAX_SETTINGS_PER_PAGE 10
#define SETTINGS_ENTRY_SPACING 3
#define SETTINGS_X_OFFSET 10
#define SETTINGS_ARROW_PREV_X 2
#define SETTINGS_ARROW_NEXT_X (RENDER_WIDTH - 5)
#define SETTINGS_GUI_TITLE_X -4
#define SETTINGS_GUI_TITLE_Y 16
#define SETTINGS_POINTER_X 7
#define SETTINGS_POINTER_DISPLAY L'▷'

//
// NEW HELPER FUNCTIONS
//
#pragma region Helper
// Converts a color code to its corresponding ANSI escape sequence.
const char* ansi_from_color(int color) {
    switch (color) {
        case COLOR_RED:
            return "\033[31m";
        case COLOR_CYAN_BOLD:
            return "\033[36;1m";
        case COLOR_CYAN:
            return "\033[36m";
        case COLOR_YELLOW:
            return "\033[33m";
        case COLOR_MAGENTA_BOLD:
            return "\033[35;1m";
        case COLOR_MAGENTA:
            return "\033[35m";
        case COLOR_GREEN:
            return "\033[32m";
        case COLOR_GRAY:
            return "\033[90m";
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

int len_int(int n) {
    char str[20];
    sprintf(str, "%d", n);
    return strlen(str);
}

static int min(int x, int y) {
    return x < y ? x : y;
}

static int max(int x, int y) {
    return x > y ? x : y;
}

//
// BOARD CREATION AND INITIALIZATION
//

// Create a board object
board create_board() {
    Cell* data = calloc(sizeof(Cell), RENDER_WIDTH * RENDER_HEIGHT);
    board b = calloc(sizeof(Cell*), RENDER_HEIGHT);
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

void clear_screen(board b) {
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        for (int j = 0; j < RENDER_WIDTH; j++) {
            if (i > 0 && (j == 0 || j == RENDER_WIDTH - 1)) {
                if (j == 0) {
                    if (i == JUNCTION_HEIGHT)
                        b[i][j].ch = L'╠';
                    else if (i == RENDER_HEIGHT - 1)
                        b[i][j].ch = L'╔';
                    else
                        b[i][j].ch = L'║';
                } else if (j == RENDER_WIDTH - 1) {
                    if (i == JUNCTION_HEIGHT)
                        b[i][j].ch = L'╣';
                    else if (i == RENDER_HEIGHT - 1)
                        b[i][j].ch = L'╗';
                    else
                        b[i][j].ch = L'║';
                }
            } else if ((i == 0 || i == RENDER_HEIGHT - 1) || i == 4) {
                if (j != 0 && j != RENDER_WIDTH - 1) {
                    b[i][j].ch = L'═';
                } else if (i == 0) {
                    if (j == 0)
                        b[i][j].ch = L'╚';
                    else
                        b[i][j].ch = L'╝';
                }
            } else {
                b[i][j].ch = L' ';
            }
            b[i][j].color = COLOR_DEFAULT;
        }
    }
}

// Draws the default screen borders and decorations.
void default_screen(board b) {
    clear_screen(b);
    for (int j = HOTBAR_START; j < HOTBAR_END; j++)
        if (j % 2 == 0) b[INFO_ROW_TOP][j].ch = L'⎹';

    write_wstr(b, INFO_ROW_TOP, 2, L"HEALTH: ", 9, COLOR_DEFAULT);
    write_wstr(b, INFO_ROW_BOTTOM, 2, L"SCORE: ", 8, COLOR_DEFAULT);
}

// Create the row changed flag array.
int* create_rc() {
    return calloc(RENDER_HEIGHT, sizeof(int));
}

// Create and initialize a new Render_Buffer.
Render_Buffer* create_screen() {
    Render_Buffer* r = malloc(sizeof(Render_Buffer));

    board b = create_board();
    clear_screen(b);

    board pv = create_board();
    blank_screen(pv);

    board dump = NULL;

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
#pragma endregion

//
// RENDERING FUNCTIONS
//
#pragma region RenderUtils
// Places a character (with default color) at a position (using center-based coordinates).
void render_char(board b, int x, int y, int c) {
    int draw_y = y + 2 + RENDER_HEIGHT / 2;
    int draw_x = x + RENDER_WIDTH / 2;
    if (draw_y >= 0 && draw_y < RENDER_HEIGHT && draw_x >= 0 && draw_x < RENDER_WIDTH) {
        b[draw_y][draw_x].ch = (wchar_t)c;
        b[draw_y][draw_x].color = COLOR_DEFAULT;
    }
}

void render_char_colored(board b, int x, int y, int c, int color) {
    int draw_y = y + 2 + RENDER_HEIGHT / 2;
    int draw_x = x + RENDER_WIDTH / 2;
    if (draw_y >= 0 && draw_y < RENDER_HEIGHT && draw_x >= 0 && draw_x < RENDER_WIDTH) {
        b[draw_y][draw_x].ch = (wchar_t)c;
        b[draw_y][draw_x].color = color;
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

// Renders a string onto the render buffer at the specified coordinates.
void render_colored_string(Render_Buffer* screen, int x, int y, char* s, int len, int color) {
    // Adjust coordinates to be relative to the center.
    int draw_x = x + RENDER_WIDTH / 2;
    int draw_y = y + 2 + RENDER_HEIGHT / 2;
    write_str(screen->bd, draw_y, draw_x, s, len, color);
}

// Renders a wide string onto the render buffer at the specified coordinates.
void render_unicode_string(Render_Buffer* screen, int x, int y, wchar_t* s, int len) {
    int draw_x = x + RENDER_WIDTH / 2;
    int draw_y = y + 2 + RENDER_HEIGHT / 2;
    write_wstr(screen->bd, draw_y, draw_x, s, len, COLOR_DEFAULT);
}

#pragma endregion
#pragma region RenderGUI

// Renders the title of an item on the screen by writing it into the board with color.
void render_item_title(Render_Buffer* screen, void* it) {
    // clear the already existing characters
    for (int i = 0; i < MAX_TITLE_SIZE; i++) {
        screen->bd[INFO_ROW_BOTTOM][TITLE_ERASE_START + i].ch = ' ';
        screen->bd[INFO_ROW_BOTTOM][TITLE_ERASE_START + i].color = COLOR_DEFAULT;
    }

    update_line(screen, 1);

    if (it == NULL) return;

    UsableItemAssetFile* uif = get_usable_item_file(get_item_usable_type((item*)it));
    char* title = uif->title;
    Rarity class = uif->specs.specs[0];

    int color = COLOR_DEFAULT;
    switch (class) {
        case RARITY_BRONZE:
            color = COLOR_RED;
            break;
        case RARITY_SILVER:
            color = COLOR_CYAN_BOLD;
            break;
        case RARITY_GOLD:
            color = COLOR_YELLOW;
            break;
        case RARITY_NADINO:
            color = COLOR_MAGENTA_BOLD;
            break;
        default:
            color = COLOR_DEFAULT;
            break;
    }

    int len = strlen(title);
    int x = (RENDER_WIDTH - len) / 2 + 2;

    write_str(screen->bd, INFO_ROW_BOTTOM, x, title, len, color);
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
        render_char_colored(b, get_item_x(it), get_item_y(it), get_item_display(it), get_item_color(it));
    }

    // Write chunk coordinates onto the board.
    int dev = len_int(get_chunk_x(c)) + len_int(get_chunk_y(c));
    int total_size = 13 + dev;
    int x_pos = CHUNK_DISPLAY_X_POS - dev;
    wchar_t tmp[total_size];
    swprintf(tmp, total_size, L"CHUNK X/Y: %d/%d", get_chunk_x(c), get_chunk_y(c));
    write_wstr(b, INFO_ROW_TOP, x_pos, tmp, total_size, COLOR_DEFAULT);
}

void render_timer(Render_Buffer* r) {
    int timer = get_time_played().tv_sec;
    int hours = timer / 3600;
    timer -= hours * 3600;
    int minutes = timer / 60;
    timer -= minutes * 60;
    int dev = len_int(hours) + len_int(minutes) + len_int(timer);
    int total_size = 18 + dev;
    int x_pos = TIMER_DISPLAY_X_POS - dev;
    wchar_t tmp[total_size];
    swprintf(tmp, total_size, L"      TIME: %dh %dm %ds", hours, minutes, timer);  // To erase previous chars when decreasing in size
    write_wstr(r->bd, INFO_ROW_MID, x_pos, tmp, total_size, COLOR_DEFAULT);
    update_line(r, INFO_ROW_MID);
}

// Renders the player onto the board.
void render_player(Render_Buffer* r, player* p) {
    render_char(r->bd, get_player_px(p), get_player_py(p), ' ');
    render_char_colored(r->bd, get_player_x(p), get_player_y(p), get_player_design(p), get_player_color(p));
}

void render_score(Render_Buffer* r, player* p) {
    int score = get_player_score(p);
    char score_str[11];
    snprintf(score_str, 11, "%d", score);
    int len = strlen(score_str);
    int display = SCORE_DISPLAY_X_POS;

    for (int i = 0; i < len; i++) {
        r->bd[INFO_ROW_BOTTOM][display].ch = score_str[i];
        r->bd[INFO_ROW_BOTTOM][display].color = COLOR_DEFAULT;
        display++;
    }

    char score_text[10];
    snprintf(score_text, 10, "/%d", ScorePerPhase[get_player_phase(p)]);
    int score_text_len = strlen(score_text);

    for (int i = 0; i < score_text_len; i++) {
        r->bd[INFO_ROW_BOTTOM][display].ch = score_text[i];
        r->bd[INFO_ROW_BOTTOM][display].color = COLOR_DEFAULT;
        display++;
    }
}

// Renders the health bar into the board.
void render_health(Render_Buffer* r, player* p) {
    int display = 10;
    int health = get_player_health(p);
    int max_health = get_player_max_health(p);
    for (int i = 0; i < max_health; i++) {
        if (i < health) {
            r->bd[INFO_ROW_TOP][display].ch = L'♥';
        } else {
            r->bd[INFO_ROW_TOP][display].ch = L'♡';
        }
        r->bd[INFO_ROW_TOP][display].color = COLOR_DEFAULT;
        display += 2;  // Spacing for hearts
    }
}

void render_mental_health(Render_Buffer* r, player* p) {
    int display = MENTAL_HEALTH_DIPLAY_X_POS;
    if (get_player_deaths(p) == 0) {
        write_wstr(r->bd, INFO_ROW_MID, 2, L"------ ------: --------", 24, COLOR_DEFAULT);
    } else {
        write_wstr(r->bd, INFO_ROW_MID, 2, L"MENTAL HEALTH: ", 16, COLOR_DEFAULT);
        int mental_health = get_player_mental_health(p);
        for (int i = 1; i <= MAX_MENTAL_HEALTH; i++) {
            if (i <= mental_health) {
                r->bd[INFO_ROW_MID][display].ch = L'█';
                r->bd[INFO_ROW_MID][display + 1].ch = L'█';
            } else {
                r->bd[INFO_ROW_MID][display].ch = ' ';
                r->bd[INFO_ROW_MID][display + 1].ch = ' ';
            }
            r->bd[INFO_ROW_MID][display].color = 1 + mental_health;
            r->bd[INFO_ROW_MID][display + 1].color = 1 + mental_health;
            display += 2;
        }
    }
}

// Renders the hotbar (including the selected item indicator) into the board.
void render_hotbar(Render_Buffer* r, hotbar* h) {
    int display = HOTBAR_DISPLAY_X_POS;
    int ms = get_hotbar_max_size(h);

    render_item_title(r, get_selected_item(h));

    for (int i = 0; i < ms; i++) {
        item* slot_item = get_hotbar(h, i);
        if (slot_item == NULL) {
            r->bd[INFO_ROW_TOP][display].ch = L' ';
            r->bd[INFO_ROW_TOP][display].color = COLOR_DEFAULT;
        } else {
            r->bd[INFO_ROW_TOP][display].ch = get_item_display(slot_item);
            r->bd[INFO_ROW_TOP][display].color = get_item_color(slot_item);
        }

        // Draw a marker above the selected slot.
        if (get_selected_slot(h) == i)
            r->bd[INFO_ROW_MID][display].ch = L'△';
        else
            r->bd[INFO_ROW_MID][display].ch = L' ';
        r->bd[INFO_ROW_MID][display].color = COLOR_DEFAULT;

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
    render_score(r, p);
    render_health(r, p);
    render_mental_health(r, p);
    render_timer(r);
}
#pragma endregion

//
// UPDATE SCREEN FUNCTIONS
//
#pragma region ScreenUpdate
void update_line_(Render_Buffer* r, int row) {
    if (!r->rc[row]) return;
    int screen_row = RENDER_HEIGHT - row;
    r->rc[row] = 0;

    int start = -1;
    int current_color = -1;

    // Buffer to accumulate characters to output.
    wchar_t buffer[RENDER_WIDTH + 1] = {0};
    for (int i = 0; i < RENDER_WIDTH; i++)
        buffer[i] = L' ';
    buffer[RENDER_WIDTH] = L'\0';

    for (int j = 0; j < RENDER_WIDTH; j++) {
        Cell curr = r->bd[row][j];
        Cell prev = r->pv[row][j];

        if (curr.ch != prev.ch || curr.color != prev.color) {
            if (start == -1) {
                start = j;
                current_color = curr.color;
            } else if (curr.color != current_color) {
                buffer[j] = L'\0';
                wprintf(L"\033[%d;%dH%s%ls", screen_row, start + 1, ansi_from_color(current_color), &buffer[start]);

                start = j;
                current_color = curr.color;
            }

            r->pv[row][j] = curr;
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
    wprintf(L"\033[%d;%dH%s", screen_row, RENDER_WIDTH, ansi_from_color(COLOR_DEFAULT));
}

// This function outputs only modified parts of the screen.
// It now also takes into account cell color changes.
void update_screen_(Render_Buffer* r) {
    for (int i = RENDER_HEIGHT - 1; i >= 0; i--) {
        if (r->rc[i])
            update_line_(r, i);
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
#pragma endregion

//
// TEXT AND CINEMATIC FUNCTIONS
//
#pragma region TextHelpers
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
    update_screen(r);
    lock_inputs();
    pause_game();
}

// Finalizes the render buffer.
void finalize_render_buffer(Render_Buffer* r) {
    free(r->bd);
    r->bd = r->dump;
    update_screen(r);
    if (GAME_PAUSED == 1) unlock_inputs();
    resume_game();
}

// No screen update variant
static void finalize_render_buffer_silent(Render_Buffer* r) {
    free(r->bd);
    r->bd = r->dump;
    if (GAME_PAUSED == 1) unlock_inputs();
    resume_game();
}

// Processes a text line (replacing characters after '~' with spaces).
void process_text_line(wchar_t* buffer, size_t width) {
    size_t len = wcslen(buffer);
    if (len > 0 && buffer[len - 1] == L'\n') buffer[--len] = L'\0';
    if (len > 0 && buffer[len - 1] == L'\r') buffer[--len] = L'\0';

    int eol = 0;
    for (size_t j = 0; j < width - 2; j++) {
        if (buffer[j] == L'~') eol = 1;
        if (eol == 1) buffer[j] = L' ';
    }
    buffer[width - 2] = L'\0';
}

// Reads text from a file into the render buffer.
void read_text_into_render(Render_Buffer* r, FILE* file) {
    wchar_t buffer[MAX_LINE];
    int i = 0;
    while (fgetws(buffer, MAX_LINE, file) != NULL && i < RENDER_HEIGHT - 2) {
        if (buffer[0] == L'#') {
            i++;
            continue;
        }
        process_text_line(buffer, RENDER_WIDTH);
        size_t len = wcslen(buffer);
        if (len > RENDER_WIDTH - 2) len = RENDER_WIDTH - 2;
        write_wstr(r->bd, RENDER_HEIGHT - i - 2, 1, buffer, len, COLOR_DEFAULT);
        i++;
    }
    for (; i < RENDER_HEIGHT - 2; i++) {
        if (i != REVERSED_INBOX_JUNCTION_HEIGHT)
            for (int j = 1; j < RENDER_WIDTH - 1; j++)
                r->bd[RENDER_HEIGHT - i - 2][j].ch = L' ';
    }
}

#pragma endregion
#pragma region MenuGUI

// Displays an item description using the render buffer.
void display_item_description(Render_Buffer* r, void* it) {
    item* item_ = (item*)it;
    if (!item_) return;

    UsableItem type = get_item_usable_type(item_);
    if (type == USABLE_ITEM_NOT_USABLE) return;

    UsableItemAssetFile* item_file = get_usable_item_file(type);
    const char* desc = item_file->description;

    setup_render_buffer(r);

    for (int i = 0; i < ITEM_DESCRIPTION_Y_OFFSET; i++) {
        for (int j = 1; j < RENDER_WIDTH - 1; j++) {
            r->bd[RENDER_HEIGHT - i - 2][j].ch = L' ';
            r->bd[RENDER_HEIGHT - i - 2][j].color = COLOR_DEFAULT;
        }
    }

    wchar_t buffer[RENDER_WIDTH - 1];
    int i = ITEM_DESCRIPTION_Y_OFFSET;

    while (fgetws_from_string(buffer, RENDER_WIDTH - 1, &desc) != NULL && i < RENDER_HEIGHT - 2) {
        process_text_line(buffer, RENDER_WIDTH);
        write_wstr(r->bd, RENDER_HEIGHT - i - 2, 1, buffer, RENDER_WIDTH - 2, COLOR_DEFAULT);
        i++;
    }

    for (; i < RENDER_HEIGHT - 2; i++) {
        if (i != REVERSED_INBOX_JUNCTION_HEIGHT)
            for (int j = 1; j < RENDER_WIDTH - 1; j++) {
                r->bd[RENDER_HEIGHT - i - 2][j].ch = L' ';
                r->bd[RENDER_HEIGHT - i - 2][j].color = COLOR_DEFAULT;
            }
    }
    render_colored_string(r, TITLE_DISPLAY_X_POS, TITLE_DISPLAY_Y_POS, " [The Eyes of The Wanderer]", 28, COLOR_YELLOW);

    wchar_t buff[60];
    swprintf(buff, 50, L"  Analysing >> %lc - %.*s : ", get_item_display(item_), (int)strlen(item_file->title) - 1, item_file->title);
    render_unicode_string(r, ANALYSIS_DISPLAY_X_POS, ANALYSIS_DISPLAY_Y_POS, buff, 50);

    render_string(r, SPACE_TO_EXIT_DISPLAY_X_POS, SPACE_TO_EXIT_DISPLAY_Y_POS, " PRESS [SPACE] TO EXIT", 23);

    update_screen(r);
    while (!USE_KEY('e') && !USE_KEY('E') && !USE_KEY('\n') && !USE_KEY(' '));

    finalize_render_buffer(r);
}

// New: display interface using interactions previously loaded with load_interactions_file()
int* display_interface_with_interactions(Render_Buffer* r, const char* visual_filename, const char* interaction_id, int* out_selected_indices) {
    return display_interface_with_interactions_main(r, visual_filename, interaction_id, out_selected_indices);
}

// draw single pattern at a board absolute pos with color application
void draw_pattern_at(Render_Buffer* r, Pos p, const char* pattern, int color_for_entire_pattern, bool use_per_char_color, int* per_char_colors, int per_char_count) {
    // pattern is an ASCII/UTF-8 string. We'll write it starting at p.x horizontally.
    // Convert the whole string to wide chars first to handle multi-byte characters properly
    int len = strlen(pattern);
    if (len == 0) return;

    // guard
    if (p.y < 0 || p.y >= RENDER_HEIGHT) return;
    int startx = p.x;
    if (startx < 0) startx = 0;

    // Convert entire pattern to wide chars at once
    wchar_t wbuffer[RENDER_WIDTH] = {0};
    size_t wlen = mbstowcs(wbuffer, pattern, RENDER_WIDTH - 1);

    // If conversion failed, try simple ASCII conversion as fallback
    if (wlen == (size_t)-1) {
        wlen = 0;
        for (int i = 0; i < len && i < RENDER_WIDTH - 1; i++) {
            wbuffer[wlen++] = (wchar_t)(unsigned char)pattern[i];
        }
    }

    // Now write each wide character with its corresponding color
    for (size_t i = 0; i < wlen; i++) {
        int tx = startx + i;
        if (tx < 0 || tx >= RENDER_WIDTH) continue;

        int color = color_for_entire_pattern;
        if (use_per_char_color && per_char_colors && i < (size_t)per_char_count) {
            color = per_char_colors[i];
        }

        r->bd[RENDER_HEIGHT - p.y][tx].ch = wbuffer[i];
        r->bd[RENDER_HEIGHT - p.y][tx].color = color;
    }
}

// Clear previous pattern at position p filling pattern length with spaces
void clear_pattern_at(Render_Buffer* r, Pos p, int pattern_len, int heavy) {
    if (p.y < 0 || p.y >= RENDER_HEIGHT) return;
    for (int i = 0; i < pattern_len - heavy; i++) {
        int tx = p.x + i;
        if (tx < 0 || tx >= RENDER_WIDTH) continue;
        r->bd[RENDER_HEIGHT - p.y][tx].ch = L' ';
        r->bd[RENDER_HEIGHT - p.y][tx].color = COLOR_DEFAULT;
    }
}

// Displays an interface (read text from a file) using the render buffer.
void display_interface(Render_Buffer* r, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    setup_render_buffer(r);

    clear_screen(r->bd);
    render_string(r, SPACE_TO_EXIT_DISPLAY_X_POS, SPACE_TO_EXIT_DISPLAY_Y_POS, " PRESS [SPACE] TO EXIT", 23);
    read_text_into_render(r, file);
    fclose(file);

    update_screen(r);

    while (!USE_KEY('\n') && !USE_KEY(' '));

    finalize_render_buffer(r);
}

// Plays a cinematic by reading lines from a file and updating the board.
void play_cinematic(Render_Buffer* r, const char* filename, int delay) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    setup_render_buffer(r);
    render_string(r, SPACE_TO_EXIT_DISPLAY_X_POS, SPACE_TO_EXIT_DISPLAY_Y_POS, " PRESS [SPACE] TO SKIP", 23);

    wchar_t buffer[MAX_LINE];
    for (int i = 0; i < RENDER_WIDTH + 4; i++) buffer[i] = L'\0';
    int row = 0;
    while (fgetws(buffer, MAX_LINE, file) != NULL && row < RENDER_HEIGHT - 2) {
        if (buffer[0] == L'#' || buffer[0] == L'§') {
            int timeout = 0;
            for (int j = 0; j < RENDER_WIDTH - 1; j++)
                if (buffer[j] == L'#') timeout++;
            for (; row < RENDER_HEIGHT - 2; row++) {
                if (row != REVERSED_INBOX_JUNCTION_HEIGHT)
                    for (int j = 1; j < RENDER_WIDTH - 1; j++) {
                        r->bd[RENDER_HEIGHT - row - 2][j].ch = L' ';
                        r->bd[RENDER_HEIGHT - row - 2][j].color = COLOR_DEFAULT;
                    }
            }
            row = 0;
            update_screen(r);
            for (int i = 0; i < timeout; i++) {
                usleep(delay);
                if (KEY_PRESSED(' ')) break;
            }
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
                if (KEY_PRESSED(' ')) break;
            }
            update_screen(r);
            row++;
        } else {
            process_text_line(buffer, RENDER_WIDTH);
            write_wstr(r->bd, RENDER_HEIGHT - row - 2, 1, buffer, wcslen(buffer), COLOR_DEFAULT);
            update_screen(r);
            row++;
        }
        if (USE_KEY(' ')) break;
    }
    USE_KEY(' ');
    finalize_render_buffer(r);
    fclose(file);
}

void display_statistics(Render_Buffer* r) {
    const int row[STATISTIC_COUNT] = {6, 9, 12, 15, 21, 18, 22, 23, 24, 25, 28};
    const int col[STATISTIC_COUNT] = {26, 25, 29, 23, 28, 25, 22, 22, 25, 24, 50};

    FILE* file = fopen("assets/interfaces/structures/statistics.dodjo", "r");
    if (!file) {
        perror("Error opening statistics file");
        return;
    }

    setup_render_buffer(r);

    read_text_into_render(r, file);
    fclose(file);

    for (int i = 0; i < STATISTIC_COUNT; i++) {
        wchar_t buffer[20];
        swprintf(buffer, 20, L"%d", get_statistic((enum StatisticID)i));
        write_wstr(r->bd, RENDER_HEIGHT - row[i] - 1, col[i], buffer, wcslen(buffer), COLOR_DEFAULT);
    }

    update_screen(r);

    while (!USE_KEY('T') && !USE_KEY('t') && !USE_KEY('\n') && !USE_KEY(' '));

    finalize_render_buffer(r);
}

void display_achievements(Render_Buffer* r, int page) {
    int max_page = (ACHIEVEMENT_COUNT - 1) / MAX_ACH_PER_PAGE;
    if (page > max_page || page < 0) return;

    clear_screen(r->pv);
    setup_render_buffer(r);

    write_wstr(r->bd, RENDER_HEIGHT / 2 + 1, ACH_ARROW_PREV_X, L" ╱", 2, page == 0 ? COLOR_GRAY : COLOR_DEFAULT);
    write_wstr(r->bd, RENDER_HEIGHT / 2, ACH_ARROW_PREV_X, L" ╲", 2, page == 0 ? COLOR_GRAY : COLOR_DEFAULT);

    write_wstr(r->bd, RENDER_HEIGHT / 2 + 1, ACH_ARROW_NEXT_X, L" ╲", 2, page == max_page ? COLOR_GRAY : COLOR_DEFAULT);
    write_wstr(r->bd, RENDER_HEIGHT / 2, ACH_ARROW_NEXT_X, L" ╱", 2, page == max_page ? COLOR_GRAY : COLOR_DEFAULT);

    for (int i = page * MAX_ACH_PER_PAGE; i < ACHIEVEMENT_COUNT && i < MAX_ACH_PER_PAGE * (page + 1); i++) {
        int j = i % MAX_ACH_PER_PAGE;
        wchar_t buffer[RENDER_WIDTH - 1];
        const char* title = get_achievement_name((enum AchievementID)i);
        int color = COLOR_RED;
        if (is_achievement_unlocked((enum AchievementID)i)) {
            swprintf(buffer, RENDER_WIDTH - 1, L"%hs: %hs", title, "UNLOCKED");
            color = COLOR_GREEN;
        } else
            swprintf(buffer, RENDER_WIDTH - 1, L"%hs: %hs (%d/%d)", title, "LOCKED", get_achievement_progress((enum AchievementID)i), get_achievement_max_progress((enum AchievementID)i));

        write_wstr(r->bd, RENDER_HEIGHT - ACH_ENTRY_SPACING * (j + 2), ACH_X_OFFSET, buffer, wcslen(buffer), color);
        swprintf(buffer, RENDER_WIDTH - 1, L"└─ %hs", get_achievement_description((enum AchievementID)i));
        write_wstr(r->bd, RENDER_HEIGHT - ACH_ENTRY_SPACING * (j + 2) - 1, ACH_X_OFFSET, buffer, wcslen(buffer), COLOR_DEFAULT);
    }
    char buffer[50];
    render_string(r, SPACE_TO_EXIT_DISPLAY_X_POS, SPACE_TO_EXIT_DISPLAY_Y_POS, " PRESS [SPACE] TO EXIT", 23);
    sprintf(buffer, "* ACHIEVEMENTS (%d/%d) *", get_completed_achiemevents(), ACHIEVEMENT_COUNT);
    render_string(r, 2 - strlen(buffer) / 2, ACH_GUI_TITLE_Y, buffer, strlen(buffer));

    update_screen(r);

    bool left = false, right = false;
    while (!USE_KEY('A') && !USE_KEY('a') && !USE_KEY('\n') && !USE_KEY(' ') && !left && !right) {
        if (page != max_page && (USE_KEY('D') || USE_KEY('d'))) right = true;
        if (page != 0 && (USE_KEY('Q') || USE_KEY('q'))) left = true;
    }

    if (left || right)
        finalize_render_buffer_silent(r);
    else
        finalize_render_buffer(r);

    if (right)
        display_achievements(r, page + 1);
    else if (left)
        display_achievements(r, page - 1);
}

void display_settings(Render_Buffer* r, int page) {
    int max_page = (SETTINGS_COUNT - 1) / MAX_SETTINGS_PER_PAGE;
    if (page > max_page || page < 0) return;

    clear_screen(r->pv);
    setup_render_buffer(r);

    write_wstr(r->bd, RENDER_HEIGHT / 2 + 1, SETTINGS_ARROW_PREV_X, L" ╱", 2, page == 0 ? COLOR_GRAY : COLOR_DEFAULT);
    write_wstr(r->bd, RENDER_HEIGHT / 2, SETTINGS_ARROW_PREV_X, L" ╲", 2, page == 0 ? COLOR_GRAY : COLOR_DEFAULT);

    write_wstr(r->bd, RENDER_HEIGHT / 2 + 1, SETTINGS_ARROW_NEXT_X, L" ╲", 2, page == max_page ? COLOR_GRAY : COLOR_DEFAULT);
    write_wstr(r->bd, RENDER_HEIGHT / 2, SETTINGS_ARROW_NEXT_X, L" ╱", 2, page == max_page ? COLOR_GRAY : COLOR_DEFAULT);

    int first_setting_on_screen = page * MAX_SETTINGS_PER_PAGE;
    int last_setting_on_screen = min(SETTINGS_COUNT, MAX_SETTINGS_PER_PAGE * (page + 1)) - 1;

    wchar_t buffer[RENDER_WIDTH - 1];
    for (int i = first_setting_on_screen; i <= last_setting_on_screen; i++) {
        int j = i % MAX_SETTINGS_PER_PAGE;
        const char* title = get_setting_name((enum SettingID)i);
        int color = get_setting_color((enum SettingID)i);

        swprintf(buffer, RENDER_WIDTH - 1, L"%hs: [%d/%d]", title, get_setting_value((enum SettingID)i), get_setting_max_value((enum SettingID)i));
        write_wstr(r->bd, RENDER_HEIGHT - SETTINGS_ENTRY_SPACING * (j + 2), SETTINGS_X_OFFSET, buffer, wcslen(buffer), color);
        swprintf(buffer, RENDER_WIDTH - 1, L"└─ %hs", get_setting_description((enum SettingID)i));
        write_wstr(r->bd, RENDER_HEIGHT - SETTINGS_ENTRY_SPACING * (j + 2) - 1, SETTINGS_X_OFFSET, buffer, wcslen(buffer), COLOR_DEFAULT);
    }
    render_string(r, SPACE_TO_EXIT_DISPLAY_X_POS, SPACE_TO_EXIT_DISPLAY_Y_POS, " PRESS [SPACE] TO EXIT", 23);
    render_string(r, SETTINGS_GUI_TITLE_X, SETTINGS_GUI_TITLE_Y, "* SETTINGS *", 13);

    write_str(r->bd, INFO_ROW_MID, (RENDER_WIDTH - 72) / 2 + 2, "Press 'P'/'M' to increment/decrement the value of the selected setting.", 72, COLOR_DEFAULT);

    int selected = first_setting_on_screen;

    r->bd[RENDER_HEIGHT - SETTINGS_ENTRY_SPACING * (selected % MAX_SETTINGS_PER_PAGE + 2)][SETTINGS_POINTER_X].ch = SETTINGS_POINTER_DISPLAY;
    update_screen(r);

    bool left = false, right = false, up = false, down = false;
    int incr = 0;

    while (!USE_KEY('\n') && !USE_KEY(' ') && !left && !right) {
        if (selected != first_setting_on_screen && (USE_KEY('Z') || USE_KEY('z'))) up = true;
        if (selected != last_setting_on_screen && (USE_KEY('S') || USE_KEY('s'))) down = true;
        if (USE_KEY('P') || USE_KEY('p')) incr = 1;
        if (USE_KEY('M') || USE_KEY('m')) incr = -1;
        if (page != max_page && (USE_KEY('D') || USE_KEY('d'))) right = true;
        if (page != 0 && (USE_KEY('Q') || USE_KEY('q'))) left = true;
        if (up || down) {
            int y = RENDER_HEIGHT - SETTINGS_ENTRY_SPACING * (selected % MAX_SETTINGS_PER_PAGE + 2);
            r->bd[y][SETTINGS_POINTER_X].ch = L' ';
            update_line(r, y);
            if (up)
                selected = max(selected - 1, first_setting_on_screen);
            else if (down)
                selected = min(selected + 1, last_setting_on_screen);
            up = false;
            down = false;
            y = RENDER_HEIGHT - SETTINGS_ENTRY_SPACING * (selected % MAX_SETTINGS_PER_PAGE + 2);
            r->bd[y][SETTINGS_POINTER_X].ch = SETTINGS_POINTER_DISPLAY;
            update_line(r, y);
        }
        if (incr) {
            if (modify_setting_value((enum SettingID)selected, incr)) {
                int j = selected % MAX_SETTINGS_PER_PAGE;
                const char* title = get_setting_name((enum SettingID)selected);
                int color = get_setting_color((enum SettingID)selected);
                int y = RENDER_HEIGHT - SETTINGS_ENTRY_SPACING * (j + 2);
                swprintf(buffer, RENDER_WIDTH - 1, L"%hs: [%d/%d]", title, get_setting_value((enum SettingID)selected), get_setting_max_value((enum SettingID)selected));
                write_wstr(r->bd, y, SETTINGS_X_OFFSET, buffer, RENDER_WIDTH - 2 - SETTINGS_X_OFFSET, color);
                update_line(r, y);
            }
            incr = 0;
        }
    }

    if (left || right)
        finalize_render_buffer_silent(r);
    else
        finalize_render_buffer(r);

    if (right)
        display_settings(r, page + 1);
    else if (left)
        display_settings(r, page - 1);
}

void home_menu(Render_Buffer* r, player* p, ResumeState resume_state) {
    if (resume_state == RESUME_DEFAULT || (resume_state == RESUME_RESET && !get_setting_value(SETTING_SKIP_START_MENU_ON_RESET)) || (resume_state == RESUME_NEW_GAME && !get_setting_value(SETTING_SKIP_START_MENU_ON_NEW_GAME))) display_interface(r, "assets/interfaces/structures/start_menu.dodjo");
    int res = 0;
    if (resume_state == RESUME_DEFAULT || (resume_state == RESUME_RESET && !get_setting_value(SETTING_SKIP_PLAYER_CUSTOM_ON_RESET)) || (resume_state == RESUME_NEW_GAME && !get_setting_value(SETTING_SKIP_PLAYER_CUSTOM_ON_NEW_GAME))) {
        int* result = display_interface_with_interactions(r, "assets/interfaces/structures/skin.dodjo", "skin", &res);
        if (result != NULL && res >= 2) {
            int class = result[0];
            int color = result[1];
            set_player_class(p, class);
            color = color == COLOR_DEFAULT ? COLOR_YELLOW : color == COLOR_YELLOW ? COLOR_DEFAULT
                                                                                  : color;
            LOG_INFO("Player changed design to %d and color to %d", get_player_design(p), color);
            set_player_color(p, color);
            free(result);
        }
    }
    res = 0;
    if (resume_state == RESUME_DEFAULT || (resume_state == RESUME_RESET && !get_setting_value(SETTING_SKIP_DIFFICULTY_SELECTION_ON_RESET)) || (resume_state == RESUME_NEW_GAME && !get_setting_value(SETTING_SKIP_DIFFICULTY_SELECTION_ON_NEW_GAME))) {
        int* result = display_interface_with_interactions(r, "assets/interfaces/structures/difficulty.dodjo", "difficulty", &res);
        if (result != NULL && res >= 1) {
            set_difficulty(result[0]);
            free(result);
        }
    }
}

ResumeState pause_menu(Render_Buffer* r, player* p, map* m, hotbar* h) {
    FILE* file = fopen("assets/interfaces/structures/pause_menu.dodjo", "r");
    if (!file) {
        perror("Error opening pause menu file");
        return RESUME_DEFAULT;
    }

    pause_game();  // Dark shenanigans
    setup_render_buffer(r);

    read_text_into_render(r, file);
    fclose(file);

    update_screen(r);

    bool no_refresh = false;
    bool loaded_a_game = false;
    ResumeState state = RESUME_DEFAULT;

    while ((!USE_KEY(' ') && !USE_KEY('\n'))) {
        if (USE_KEY('N') || USE_KEY('n')) {
            write_str(r->bd, INFO_ROW_MID, 2, " ", RENDER_WIDTH - 4, COLOR_DEFAULT);
            if (save_game("assets/data/save.dat", p, m, h)) {
                LOG_INFO("Game saved successfully!");
                write_str(r->bd, INFO_ROW_MID, (RENDER_WIDTH - 24) / 2 + 2, "Game saved successfully!", 25, COLOR_GREEN);
                play_sound_effect_by_id(AUDIO_SUCCESS);
            } else {
                LOG_ERROR("Failed to save game");
                write_str(r->bd, INFO_ROW_MID, (RENDER_WIDTH - 19) / 2 + 1, "Failed to save game", 20, COLOR_RED);
                play_sound_effect_by_id(AUDIO_FAILURE);
            }
            update_screen(r);
        } else if (USE_KEY('B') || USE_KEY('b')) {
            write_str(r->bd, INFO_ROW_MID, 2, " ", RENDER_WIDTH - 4, COLOR_DEFAULT);
            if (load_game("assets/data/save.dat", p, m, h)) {
                loaded_a_game = true;
                LOG_INFO("Game loaded successfully!");
                write_str(r->bd, INFO_ROW_MID, (RENDER_WIDTH - 25) / 2 + 1, "Game loaded successfully!", 26, COLOR_GREEN);
                play_sound_effect_by_id(AUDIO_SUCCESS);
            } else {
                LOG_ERROR("Failed to load game");
                write_str(r->bd, INFO_ROW_MID, (RENDER_WIDTH - 19) / 2 + 1, "Failed to load game", 20, COLOR_RED);
                play_sound_effect_by_id(AUDIO_FAILURE);
            }
            update_screen(r);
        } else if (USE_KEY('H') || USE_KEY('h')) {
            display_interface(r, "assets/interfaces/structures/help.dodjo");
            no_refresh = true;
        } else if (USE_KEY('A') || USE_KEY('a')) {
            display_achievements(r, 0);
            no_refresh = true;
        } else if (USE_KEY('T') || USE_KEY('t')) {
            display_statistics(r);
            no_refresh = true;
        } else if (USE_KEY('S') || USE_KEY('s')) {
            display_settings(r, 0);
            no_refresh = true;
        } else if (USE_KEY('R') || USE_KEY('r')) {
            state = RESUME_RESET;
            break;
        } else if (USE_KEY('G') || USE_KEY('g')) {
            state = RESUME_NEW_GAME;
            break;
        }
        struct timespec ts = {.tv_sec = 0, .tv_nsec = 50000000};
        nanosleep(&ts, NULL);
    }

    if (!no_refresh)
        finalize_render_buffer(r);
    if (no_refresh || loaded_a_game) {
        render(r, m);
        update_screen(r);
        resume_game();
    }

    unlock_inputs();
    resume_game();
    return state;
}

#pragma endregion

// Mark rows as changed and update the screen.
void update_line(Render_Buffer* r, int row) {
    mark_changed_rows(r);
    update_line_(r, row);
    fflush(stdout);
}

// Mark rows as changed and update the screen.
void update_screen(Render_Buffer* r) {
    mark_changed_rows(r);
    update_screen_(r);
}
