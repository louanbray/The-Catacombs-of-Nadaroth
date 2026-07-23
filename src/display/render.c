#include "render.h"

#include <omp.h>
#include <time.h>

#include "../game_objects/item.h"
#include "../game_objects/map.h"
#include "../game_objects/player.h"
#include "../managers/managers.h"
#include "../utils/dynarray.h"
#include "../utils/game_status.h"
#include "../utils/logger.h"
#include "../utils/sys_platform.h"

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

static bool FOG_OF_WAR = false;
static bool IN_MENU = false;
static int FOG_OF_WAR_ORIGIN_X = WTR_X(0);
static int FOG_OF_WAR_ORIGIN_Y = WTR_Y(0);
static int FOG_OF_WAR_CLEAR_VISION_RADIUS = (6 * 6);  // 6
static int FOG_OF_WAR_FOG_RADIUS = (9 * 9);           // 9

#define INFO_ROW_BOTTOM (RENDER_HEIGHT - 2)
#define INFO_ROW_MID (RENDER_HEIGHT - 3)
#define INFO_ROW_TOP (RENDER_HEIGHT - 4)

#define MAX_LINE 512

// For the outline box
#define JUNCTION_HEIGHT HUD_HEIGHT
#define REVERSED_INBOX_JUNCTION_HEIGHT (RENDER_HEIGHT - JUNCTION_HEIGHT - 2)

#define HOTBAR_SPACING 4
#define HOTBAR_START ((RENDER_WIDTH + 1) / 2 - (5 * HOTBAR_SPACING))
#define HOTBAR_END ((RENDER_WIDTH + 1) / 2 + (5 * HOTBAR_SPACING))

#define KEYHOLDER_START (HOTBAR_END + 6)

#define TITLE_ERASE_START (RENDER_WIDTH / 2 - 10)
#define MAX_TITLE_SIZE 20

#define CHUNK_DISPLAY_X_POS (RENDER_WIDTH - 14)
#define TIMER_DISPLAY_X_POS (RENDER_WIDTH - 19)
#define SCORE_DISPLAY_X_POS 9
#define MENTAL_HEALTH_DIPLAY_X_POS 17
#define HOTBAR_DISPLAY_X_POS (HOTBAR_START + HOTBAR_SPACING)

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
const wchar_t* ansi_from_color(int color) {
    switch (color) {
        case COLOR_RED:
            return L"\033[31m";
        case COLOR_CYAN_BOLD:
            return L"\033[36;1m";
        case COLOR_CYAN:
            return L"\033[36m";
        case COLOR_YELLOW:
            return L"\033[33m";
        case COLOR_MAGENTA_BOLD:
            return L"\033[35;1m";
        case COLOR_MAGENTA:
            return L"\033[35m";
        case COLOR_GREEN:
            return L"\033[32m";
        case COLOR_GRAY:
            return L"\033[90m";
        case COLOR_DEFAULT:
        default:
            return L"\033[0m";
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
void free_board(board b) {
    if (b == NULL) return;
    free(b[0]);
    free(b);
}

void free_render_buffer(Render_Buffer* screen) {
    if (!screen) return;
    if (screen->dump == screen->bd) screen->dump = NULL;
    free_board(screen->bd);
    free_board(screen->pv);
    free_board(screen->dump);
    free(screen->rc);
    free(screen);
}

board create_board() {
    Cell* data = calloc(RENDER_WIDTH * RENDER_HEIGHT, sizeof(Cell));
    board b = calloc(RENDER_HEIGHT, sizeof(Cell*));
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
            if (i < RENDER_HEIGHT - 1 && (j == 0 || j == RENDER_WIDTH - 1)) {
                if (j == 0) {
                    if (i == RENDER_HEIGHT - 1 - JUNCTION_HEIGHT)
                        b[i][j].ch = L'╠';
                    else if (i == 0)
                        b[i][j].ch = L'╔';
                    else
                        b[i][j].ch = L'║';
                } else if (j == RENDER_WIDTH - 1) {
                    if (i == RENDER_HEIGHT - 1 - JUNCTION_HEIGHT)
                        b[i][j].ch = L'╣';
                    else if (i == 0)
                        b[i][j].ch = L'╗';
                    else
                        b[i][j].ch = L'║';
                }
            } else if ((i == 0 || i == RENDER_HEIGHT - 1) || i == RENDER_HEIGHT - 1 - JUNCTION_HEIGHT) {
                if (j != 0 && j != RENDER_WIDTH - 1) {
                    b[i][j].ch = L'═';
                } else if (i == RENDER_HEIGHT - 1) {
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
        if (j % HOTBAR_SPACING == 0) b[INFO_ROW_TOP][j].ch = L'⎹';

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
    int draw_y = WTR_Y(y);
    int draw_x = WTR_X(x);
    if (draw_y >= 0 && draw_y < RENDER_HEIGHT && draw_x >= 0 && draw_x < RENDER_WIDTH) {
        b[draw_y][draw_x].ch = (wchar_t)c;
        b[draw_y][draw_x].color = COLOR_DEFAULT;
    }
}

void render_char_colored(board b, int x, int y, int c, int color) {
    int draw_y = WTR_Y(y);
    int draw_x = WTR_X(x);
    if (draw_y >= 0 && draw_y < RENDER_HEIGHT && draw_x >= 0 && draw_x < RENDER_WIDTH) {
        b[draw_y][draw_x].ch = (wchar_t)c;
        b[draw_y][draw_x].color = color;
    }
}

wchar_t render_get_cell_char(Render_Buffer* screen, int y, int x) {
    return screen->bd[y][x].ch;
}

void render_set_cell_char(Render_Buffer* screen, int y, int x, int c) {
    if (GAME_PAUSED) return;
    screen->bd[y][x].ch = c;
}

// Renders a string onto the render buffer at the specified coordinates.
void render_string(Render_Buffer* screen, int x, int y, char* s, int len) {
    write_str(screen->bd, WTR_Y(y), WTR_X(x), s, len, COLOR_DEFAULT);
}

// Renders a string onto the render buffer at the specified coordinates.
void render_colored_string(Render_Buffer* screen, int x, int y, char* s, int len, int color) {
    write_str(screen->bd, WTR_Y(y), WTR_X(x), s, len, color);
}

// Renders a wide string onto the render buffer at the specified coordinates.
void render_unicode_string(Render_Buffer* screen, int x, int y, wchar_t* s, int len) {
    write_wstr(screen->bd, WTR_Y(y), WTR_X(x), s, len, COLOR_DEFAULT);
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
    board b = r->bd;
    dynarray* d = get_chunk_furniture_list(c);
    int len = len_dyn(d);

    chunk_render_walls(c, b);

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
}

void render_current_chunk_name(Render_Buffer* r, player* p) {
    chunk* current_chunk = get_player_chunk(p);
    if (!current_chunk) return;
    const char* name = CHUNK_NAMES[get_chunk_type(current_chunk)];
    write_str(r->bd, INFO_ROW_BOTTOM, RENDER_WIDTH - 2 - strlen(name), name, strlen(name), COLOR_DEFAULT);
}

// Renders the player onto the board.
void render_player(Render_Buffer* r, player* p) {
    if (r->bd[WTR_Y(get_player_py(p))][WTR_X(get_player_px(p))].ch == get_player_design(p)) render_char(r->bd, get_player_px(p), get_player_py(p), ' ');
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

        display += HOTBAR_SPACING;
    }
}

void render_keyholder(Render_Buffer* r, keyholder* k) {
    int display = KEYHOLDER_START;
    KeyHolderLevel level = get_keyholder_level(k);
    for (Rarity rarity = 0; rarity < RARITY_COUNT; rarity++) {
        Color color = get_color_for_rarity(rarity);
        bool draw = (int)rarity < (int)level;
        r->bd[INFO_ROW_TOP][display].ch = draw ? CHEST_KEY_DESIGN : L' ';
        r->bd[INFO_ROW_TOP][display].color = color;

        int key_nb = get_keyholder_keys_of_rarity(k, rarity);
        r->bd[INFO_ROW_MID][display].ch = draw ? min(9, key_nb) + L'0' : L' ';
        r->bd[INFO_ROW_MID][display].color = color;

        r->bd[INFO_ROW_MID][display + 1].ch = draw && key_nb > 9 ? L'+' : L' ';
        r->bd[INFO_ROW_MID][display + 1].color = color;

        display += 3;
    }
}

void fog_of_war_setup(int vision_radius, int fog_radius) {
    FOG_OF_WAR_CLEAR_VISION_RADIUS = vision_radius * vision_radius;
    FOG_OF_WAR_FOG_RADIUS = fog_radius * fog_radius;
}
void fog_of_war_set_origin(int x, int y) {
    FOG_OF_WAR_ORIGIN_X = WTR_X(x);
    FOG_OF_WAR_ORIGIN_Y = WTR_Y(y);
}

void fog_of_war_enable() {
    FOG_OF_WAR = true;
}

void fog_of_war_disable() {
    FOG_OF_WAR = false;
}

bool has_fog_of_war() {
    return FOG_OF_WAR;
}

void fog_of_war_toggle() {
    FOG_OF_WAR = !FOG_OF_WAR;
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
    render_keyholder(r, get_player_keyholder(p));
    render_current_chunk_name(r, p);
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
    IN_MENU = true;
    if (r->dump) free_board(r->dump);
    r->dump = r->bd;
    r->bd = r->pv;
    r->pv = create_board();
    clear_screen(r->pv);
    update_screen(r);
    lock_inputs();
    pause_game();
}

// Finalizes the render buffer.
void finalize_render_buffer(Render_Buffer* r) {
    IN_MENU = false;
    free_board(r->bd);
    r->bd = r->dump;
    r->dump = NULL;
    update_screen(r);
    if (GAME_PAUSED == 1) unlock_inputs();
    resume_game();
}

// No screen update variant
void finalize_render_buffer_silent(Render_Buffer* r) {
    IN_MENU = false;
    free_board(r->bd);
    r->bd = r->dump;
    r->dump = NULL;
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
    wchar_t buffer[MAX_LINE] = {0};
    int i = 0;
    while (is_game_running() && portable_fgetws(buffer, MAX_LINE, file) != NULL && i < RENDER_HEIGHT - 2) {
        if (buffer[0] == L'#') {
            i++;
            continue;
        }
        process_text_line(buffer, RENDER_WIDTH);
        size_t len = wcslen(buffer);
        if (len > RENDER_WIDTH - 2) len = RENDER_WIDTH - 2;
        write_wstr(r->bd, i + 1, 1, buffer, len, COLOR_DEFAULT);
        i++;
    }
    for (; i < RENDER_HEIGHT - 2; i++) {
        if (i != REVERSED_INBOX_JUNCTION_HEIGHT)
            for (int j = 1; j < RENDER_WIDTH - 1; j++)
                r->bd[i + 1][j].ch = L' ';
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
            r->bd[i + 1][j].ch = L' ';
            r->bd[i + 1][j].color = COLOR_DEFAULT;
        }
    }

    wchar_t buffer[RENDER_WIDTH - 1];
    int i = ITEM_DESCRIPTION_Y_OFFSET;

    while (is_game_running() && fgetws_from_string(buffer, RENDER_WIDTH - 1, &desc) != NULL && i < RENDER_HEIGHT - 2) {
        process_text_line(buffer, RENDER_WIDTH);
        write_wstr(r->bd, i + 1, 1, buffer, RENDER_WIDTH - 2, COLOR_DEFAULT);
        i++;
    }

    for (; i < RENDER_HEIGHT - 2; i++) {
        if (i != REVERSED_INBOX_JUNCTION_HEIGHT)
            for (int j = 1; j < RENDER_WIDTH - 1; j++) {
                r->bd[i + 1][j].ch = L' ';
                r->bd[i + 1][j].color = COLOR_DEFAULT;
            }
    }
    render_colored_string(r, TITLE_DISPLAY_X_POS, TITLE_DISPLAY_Y_POS, " [The Eyes of The Wanderer]", 28, COLOR_YELLOW);

    wchar_t buff[60];
    swprintf(buff, 50, L"  Analysing >> %lc - %.*s : ", get_item_display(item_), (int)strlen(item_file->title) - 1, item_file->title);
    render_unicode_string(r, ANALYSIS_DISPLAY_X_POS, ANALYSIS_DISPLAY_Y_POS, buff, 50);

    render_string(r, SPACE_TO_EXIT_DISPLAY_X_POS, SPACE_TO_EXIT_DISPLAY_Y_POS, " PRESS [SPACE] TO EXIT", 23);

    update_screen(r);
    while (!USE_KEY('e') && !USE_KEY('E') && !USE_KEY('\n') && !USE_KEY(' ')) sys_sleep_ms(50);

    finalize_render_buffer(r);
}

// New: display interface using interactions previously loaded with load_interactions_file()
int* display_interface_with_interactions(Render_Buffer* r, const char* visual_filename, const char* interaction_id, int* out_selected_indices) {
    return display_interface_with_interactions_main(r, visual_filename, interaction_id, out_selected_indices);
}

// draw single pattern at a board absolute pos with color application
void draw_pattern_at(Render_Buffer* r, Pos p, const char* pattern, int color_for_entire_pattern, bool use_per_char_color, int* per_char_colors, int per_char_count) {
    int len = strlen(pattern);
    if (len == 0) return;

    // Guard
    if (p.y < 0 || p.y >= RENDER_HEIGHT) return;
    int startx = p.x;
    if (startx < 0) startx = 0;

    wchar_t wbuffer[RENDER_WIDTH] = {0};
    size_t wlen = 0;

    // Décodage manuel UTF-8 : Garantit la reconstruction de '■' sans mbstowcs
    size_t i = 0;
    while (i < (size_t)len && wlen < RENDER_WIDTH - 1) {
        unsigned char c = pattern[i];
        wchar_t wc = 0;
        int n = 0;

        if (c < 0x80) {
            wc = c;
            n = 1;
        } else if ((c & 0xE0) == 0xC0) {
            wc = c & 0x1F;
            n = 2;
        } else if ((c & 0xF0) == 0xE0) {
            wc = c & 0x0F;
            n = 3;
        } else if ((c & 0xF8) == 0xF0) {
            wc = c & 0x07;
            n = 4;
        } else {
            wc = c;
            n = 1;
        }  // Fallback octet simple

        if (i + n > (size_t)len) n = len - i;

        // Reconstitution du caractère Unicode
        for (int j = 1; j < n; j++) {
            wc = (wc << 6) | (pattern[i + j] & 0x3F);
        }

        wbuffer[wlen++] = wc;
        i += n;
    }

    // Écriture de chaque caractère large avec sa couleur associée
    for (i = 0; i < wlen; i++) {
        int tx = startx + i;
        if (tx < 0 || tx >= RENDER_WIDTH) continue;

        int color = color_for_entire_pattern;
        if (use_per_char_color && per_char_colors && i < (size_t)per_char_count) {
            color = per_char_colors[i];
        }

        r->bd[p.y - 1][tx].ch = wbuffer[i];
        r->bd[p.y - 1][tx].color = color;
    }
}

// Clear previous pattern at position p filling pattern length with spaces
void clear_pattern_at(Render_Buffer* r, Pos p, int pattern_len) {
    if (p.y < 0 || p.y >= RENDER_HEIGHT) return;

    // On retire "- heavy" car pattern_len est désormais le nombre exact
    // de colonnes réelles à effacer à l'écran.
    for (int i = 0; i < pattern_len; i++) {
        int tx = p.x + i;
        if (tx < 0 || tx >= RENDER_WIDTH) continue;
        r->bd[p.y - 1][tx].ch = L' ';
        r->bd[p.y - 1][tx].color = COLOR_DEFAULT;
    }
}

// Displays an interface (read text from a file) using the render buffer.
void display_interface(Render_Buffer* r, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        LOG_ERROR("Error opening file.");
        return;
    }

    setup_render_buffer(r);

    clear_screen(r->bd);
    render_string(r, SPACE_TO_EXIT_DISPLAY_X_POS, SPACE_TO_EXIT_DISPLAY_Y_POS, " PRESS [SPACE] TO EXIT", 23);
    read_text_into_render(r, file);
    fclose(file);

    update_screen(r);

    while (!USE_KEY('\n') && !USE_KEY(' ')) sys_sleep_ms(50);

    finalize_render_buffer(r);
}

// Plays a cinematic by reading lines from a file and updating the board.
void play_cinematic(Render_Buffer* r, const char* filename, int delay) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        LOG_ERROR("Error opening file.");
        return;
    }
    setup_render_buffer(r);
    render_string(r, SPACE_TO_EXIT_DISPLAY_X_POS, SPACE_TO_EXIT_DISPLAY_Y_POS, " PRESS [SPACE] TO SKIP", 23);

    wchar_t buffer[MAX_LINE];
    for (int i = 0; i < RENDER_WIDTH + 4; i++) buffer[i] = L'\0';
    int row = 0;
    while (is_game_running() && portable_fgetws(buffer, MAX_LINE, file) != NULL && row < RENDER_HEIGHT - 2) {
        if (buffer[0] == L'#' || buffer[0] == L'§') {
            int timeout = 0;
            for (int j = 0; j < RENDER_WIDTH - 1; j++)
                if (buffer[j] == L'#') timeout++;
            for (; row < RENDER_HEIGHT - 2; row++) {
                if (row != REVERSED_INBOX_JUNCTION_HEIGHT)
                    for (int j = 1; j < RENDER_WIDTH - 1; j++) {
                        r->bd[row + 1][j].ch = L' ';
                        r->bd[row + 1][j].color = COLOR_DEFAULT;
                    }
            }
            row = 0;
            update_screen(r);
            for (int i = 0; i < timeout; i++) {
                sys_sleep_ms(delay);
                if (!is_game_running() || KEY_PRESSED(' ')) break;
            }
        } else if (buffer[0] == L'%') {
            int eol = 0;
            for (int j = 1; j < RENDER_WIDTH - 2; j++) {
                if (buffer[j] == L'~') eol = 1;
                if (eol == 1) buffer[j] = L' ';
                r->bd[row + 1][j].ch = buffer[j];
                r->bd[row + 1][j].color = COLOR_DEFAULT;
                if (!eol) {
                    update_screen(r);
                    sys_sleep_ms(delay / 50);
                }
                if (!is_game_running() || KEY_PRESSED(' ')) break;
            }
            update_screen(r);
            row++;
        } else {
            process_text_line(buffer, RENDER_WIDTH);
            write_wstr(r->bd, row + 1, 1, buffer, wcslen(buffer), COLOR_DEFAULT);
            update_screen(r);
            row++;
        }
        if (USE_KEY(' ')) break;
    }
    USE_KEY(' ');
    if (!is_game_running()) unlock_inputs();
    finalize_render_buffer_silent(r);
    fclose(file);
}

void display_statistics(Render_Buffer* r) {
    const int row[STATISTIC_COUNT] = {6, 9, 12, 15, 21, 18, 22, 23, 24, 25, 28};
    const int col[STATISTIC_COUNT] = {26, 25, 29, 23, 28, 25, 22, 22, 25, 24, 50};

    FILE* file = fopen("assets/interfaces/structures/statistics.dodjo", "r");
    if (!file) {
        perror("Error opening statistics file");
        LOG_ERROR("Error opening statistics file.");
        return;
    }

    setup_render_buffer(r);

    read_text_into_render(r, file);
    fclose(file);

    for (int i = 0; i < STATISTIC_COUNT; i++) {
        wchar_t buffer[20];
        swprintf(buffer, 20, L"%d", get_statistic((enum StatisticID)i));
        write_wstr(r->bd, row[i], col[i], buffer, wcslen(buffer), COLOR_DEFAULT);
    }

    update_screen(r);

    while (!USE_KEY('T') && !USE_KEY('t') && !USE_KEY('\n') && !USE_KEY(' ')) sys_sleep_ms(50);

    finalize_render_buffer(r);
}

void display_achievements(Render_Buffer* r, int page) {
    int max_page = (ACHIEVEMENT_COUNT - 1) / MAX_ACH_PER_PAGE;
    if (page > max_page || page < 0) return;

    clear_screen(r->pv);
    setup_render_buffer(r);

    write_wstr(r->bd, RENDER_HEIGHT / 2 - 1, ACH_ARROW_PREV_X, L" ╱", 2, page == 0 ? COLOR_GRAY : COLOR_DEFAULT);
    write_wstr(r->bd, RENDER_HEIGHT / 2, ACH_ARROW_PREV_X, L" ╲", 2, page == 0 ? COLOR_GRAY : COLOR_DEFAULT);

    write_wstr(r->bd, RENDER_HEIGHT / 2 - 1, ACH_ARROW_NEXT_X, L" ╲", 2, page == max_page ? COLOR_GRAY : COLOR_DEFAULT);
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

        write_wstr(r->bd, ACH_ENTRY_SPACING * (j + 2) - 1, ACH_X_OFFSET, buffer, wcslen(buffer), color);
        swprintf(buffer, RENDER_WIDTH - 1, L"└─ %hs", get_achievement_description((enum AchievementID)i));
        write_wstr(r->bd, ACH_ENTRY_SPACING * (j + 2), ACH_X_OFFSET, buffer, wcslen(buffer), COLOR_DEFAULT);
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
        sys_sleep_ms(50);
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

    write_wstr(r->bd, RENDER_HEIGHT / 2 - 1, SETTINGS_ARROW_PREV_X, L" ╱", 2, page == 0 ? COLOR_GRAY : COLOR_DEFAULT);
    write_wstr(r->bd, RENDER_HEIGHT / 2, SETTINGS_ARROW_PREV_X, L" ╲", 2, page == 0 ? COLOR_GRAY : COLOR_DEFAULT);

    write_wstr(r->bd, RENDER_HEIGHT / 2 - 1, SETTINGS_ARROW_NEXT_X, L" ╲", 2, page == max_page ? COLOR_GRAY : COLOR_DEFAULT);
    write_wstr(r->bd, RENDER_HEIGHT / 2, SETTINGS_ARROW_NEXT_X, L" ╱", 2, page == max_page ? COLOR_GRAY : COLOR_DEFAULT);

    int first_setting_on_screen = page * MAX_SETTINGS_PER_PAGE;
    int last_setting_on_screen = min(SETTINGS_COUNT, MAX_SETTINGS_PER_PAGE * (page + 1)) - 1;

    wchar_t buffer[RENDER_WIDTH - 1];
    for (int i = first_setting_on_screen; i <= last_setting_on_screen; i++) {
        int j = i % MAX_SETTINGS_PER_PAGE;
        const char* title = get_setting_name((enum SettingID)i);
        int color = get_setting_color((enum SettingID)i);

        swprintf(buffer, RENDER_WIDTH - 1, L"%hs: [%d/%d]", title, get_setting_value((enum SettingID)i), get_setting_max_value((enum SettingID)i));
        write_wstr(r->bd, SETTINGS_ENTRY_SPACING * (j + 2) - 1, SETTINGS_X_OFFSET, buffer, wcslen(buffer), color);
        swprintf(buffer, RENDER_WIDTH - 1, L"└─ %hs", get_setting_description((enum SettingID)i));
        write_wstr(r->bd, SETTINGS_ENTRY_SPACING * (j + 2), SETTINGS_X_OFFSET, buffer, wcslen(buffer), COLOR_DEFAULT);
    }
    render_string(r, SPACE_TO_EXIT_DISPLAY_X_POS, SPACE_TO_EXIT_DISPLAY_Y_POS, " PRESS [SPACE] TO EXIT", 23);
    render_string(r, SETTINGS_GUI_TITLE_X, SETTINGS_GUI_TITLE_Y, "* SETTINGS *", 13);

    write_str(r->bd, INFO_ROW_MID, (RENDER_WIDTH - 72) / 2 + 2, "Press 'P'/'M' to increment/decrement the value of the selected setting.", 72, COLOR_DEFAULT);

    int selected = first_setting_on_screen;

    r->bd[SETTINGS_ENTRY_SPACING * (selected % MAX_SETTINGS_PER_PAGE + 2) - 1][SETTINGS_POINTER_X].ch = SETTINGS_POINTER_DISPLAY;
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
            int y = SETTINGS_ENTRY_SPACING * (selected % MAX_SETTINGS_PER_PAGE + 2) - 1;
            r->bd[y][SETTINGS_POINTER_X].ch = L' ';
            update_screen(r);
            if (up)
                selected = max(selected - 1, first_setting_on_screen);
            else if (down)
                selected = min(selected + 1, last_setting_on_screen);
            up = false;
            down = false;
            y = SETTINGS_ENTRY_SPACING * (selected % MAX_SETTINGS_PER_PAGE + 2) - 1;
            r->bd[y][SETTINGS_POINTER_X].ch = SETTINGS_POINTER_DISPLAY;
            update_screen(r);
        }
        if (incr) {
            if (modify_setting_value((enum SettingID)selected, incr)) {
                int j = selected % MAX_SETTINGS_PER_PAGE;
                const char* title = get_setting_name((enum SettingID)selected);
                int color = get_setting_color((enum SettingID)selected);
                int y = SETTINGS_ENTRY_SPACING * (j + 2) - 1;
                swprintf(buffer, RENDER_WIDTH - 1, L"%hs: [%d/%d]", title, get_setting_value((enum SettingID)selected), get_setting_max_value((enum SettingID)selected));
                write_wstr(r->bd, y, SETTINGS_X_OFFSET, buffer, RENDER_WIDTH - 2 - SETTINGS_X_OFFSET, color);
                update_screen(r);
            }
            incr = 0;
        }
        sys_sleep_ms(50);
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
        LOG_ERROR("Error opening pause menu file.");
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
            if (save_game("saves/user_saves/0.dat", p, m, h)) {
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
            kill_all_projectiles(r);
            if (load_game("saves/user_saves/0.dat", p, m, h)) {
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
        sys_sleep_ms(50);
    }

    if (!no_refresh)
        finalize_render_buffer(r);
    if (no_refresh || loaded_a_game) {
        IN_MENU = false;
        render(r, m);
        update_screen(r);
        resume_game();
    }

    unlock_inputs();
    resume_game();
    return state;
}

#pragma endregion

#pragma region ScreenUpdate
static Cell blank_cell = {.ch = L' ', .color = COLOR_DEFAULT};
static int glitch_ticks_left = 0;
static int glitch_chance = 5;
#ifdef _WIN32

static const WORD WIN32_COLOR_LOOKUP[] = {
    [COLOR_DEFAULT] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    [COLOR_RED] = FOREGROUND_RED,
    [COLOR_CYAN_BOLD] = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    [COLOR_CYAN] = FOREGROUND_GREEN | FOREGROUND_BLUE,
    [COLOR_YELLOW] = FOREGROUND_RED | FOREGROUND_GREEN,
    [COLOR_MAGENTA_BOLD] = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    [COLOR_MAGENTA] = FOREGROUND_RED | FOREGROUND_BLUE,
    [COLOR_GREEN] = FOREGROUND_GREEN,
    [COLOR_GRAY] = FOREGROUND_INTENSITY,
};

static HANDLE hOut = INVALID_HANDLE_VALUE;

// Uses Windows API
void update_screen(Render_Buffer* r) {
    static CHAR_INFO bufferA[RENDER_WIDTH * RENDER_HEIGHT];
    static CHAR_INFO bufferB[RENDER_WIDTH * RENDER_HEIGHT];
    static CHAR_INFO* cur = bufferA;
    static CHAR_INFO* prev = bufferB;
    static BOOL first_frame = TRUE;

    if (hOut == INVALID_HANDLE_VALUE) {
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    int minRow = RENDER_HEIGHT, maxRow = -1;
    int minCol = RENDER_WIDTH, maxCol = -1;

    bool is_glitching = (glitch_ticks_left > 0);
    if (glitch_ticks_left > 0) glitch_ticks_left--;

    for (int i = 0; i < RENDER_HEIGHT; i++) {
        for (int j = 0; j < RENDER_WIDTH; j++) {
            int idx = i * RENDER_WIDTH + j;

            const Cell* c = &r->bd[i][j];
            if (is_glitching && (rand() % 100) < glitch_chance) c = &blank_cell;

            wchar_t ch = c->ch;
            Color color = c->color;

            // ---- Fog of War ----
            if (FOG_OF_WAR && !IN_MENU && i > 0 && i <= REVERSED_INBOX_JUNCTION_HEIGHT && j > 0 && j < RENDER_WIDTH - 1) {
                int dx = (j - FOG_OF_WAR_ORIGIN_X) / 2;
                int dy = i - FOG_OF_WAR_ORIGIN_Y;
                int dist_sq = (dx * dx) + (dy * dy);

                if (dist_sq > FOG_OF_WAR_FOG_RADIUS) {
                    ch = L' ';
                    color = COLOR_DEFAULT;
                } else if (dist_sq > FOG_OF_WAR_CLEAR_VISION_RADIUS) {
                    color = COLOR_GRAY;
                    ch = ch == L' ' ? L' ' : L'░';
                }
            }

            WORD attr = WIN32_COLOR_LOOKUP[color];

            if (first_frame ||
                ch != prev[idx].Char.UnicodeChar ||
                attr != prev[idx].Attributes) {
                if (i < minRow) minRow = i;
                if (i > maxRow) maxRow = i;
                if (j < minCol) minCol = j;
                if (j > maxCol) maxCol = j;
            }

            cur[idx].Char.UnicodeChar = ch;
            cur[idx].Attributes = attr;
        }
    }

    if (maxRow >= 0) {
        COORD buffer_size = {(SHORT)RENDER_WIDTH, (SHORT)RENDER_HEIGHT};
        COORD buffer_coord = {(SHORT)minCol, (SHORT)minRow};
        SMALL_RECT write_region = {
            (SHORT)minCol, (SHORT)minRow, (SHORT)maxCol, (SHORT)maxRow};

        WriteConsoleOutputW(hOut, cur, buffer_size, buffer_coord, &write_region);
    }

    CHAR_INFO* tmp = cur;
    cur = prev;
    prev = tmp;
    first_frame = FALSE;
}

#else
// VERSION LINUX / WSL (Conversion UTF-8 manuelle ultra-rapide en RAM)
static inline void append_char_utf8(char* dest, int* idx, wchar_t cp) {
    if (cp < 0x80) {
        dest[(*idx)++] = (char)cp;
    } else if (cp < 0x800) {
        dest[(*idx)++] = (char)((cp >> 6) | 0xC0);
        dest[(*idx)++] = (char)((cp & 0x3F) | 0x80);
    } else if (cp < 0x10000) {
        dest[(*idx)++] = (char)((cp >> 12) | 0xE0);
        dest[(*idx)++] = (char)(((cp >> 6) & 0x3F) | 0x80);
        dest[(*idx)++] = (char)((cp & 0x3F) | 0x80);
    } else if (cp < 0x110000) {
        dest[(*idx)++] = (char)((cp >> 18) | 0xF0);
        dest[(*idx)++] = (char)(((cp >> 12) & 0x3F) | 0x80);
        dest[(*idx)++] = (char)(((cp >> 6) & 0x3F) | 0x80);
        dest[(*idx)++] = (char)((cp & 0x3F) | 0x80);
    }
}

static inline void append_ansi_utf8(char* dest, int* idx, const wchar_t* src) {
    while (*src) {
        dest[(*idx)++] = (char)*src++;
    }
}

void update_screen(Render_Buffer* r) {
    static char frame_buffer[RENDER_WIDTH * RENDER_HEIGHT * 32 + 4096];
    int out_idx = 0;

    bool is_glitching = (glitch_ticks_left > 0);
    if (glitch_ticks_left > 0) glitch_ticks_left--;

    append_ansi_utf8(frame_buffer, &out_idx, L"\033[H");

    int current_color = -1;

    for (int i = 0; i < RENDER_HEIGHT; i++) {
        for (int j = 0; j < RENDER_WIDTH; j++) {
            const Cell* c = &r->bd[i][j];
            if (is_glitching && (rand() % 100) < glitch_chance) c = &blank_cell;

            wchar_t ch = c->ch;
            int color = c->color;

            // ---- Fog of War ----
            if (FOG_OF_WAR && !IN_MENU && i > 0 && i <= REVERSED_INBOX_JUNCTION_HEIGHT && j > 0 && j < RENDER_WIDTH - 1) {
                int dx = (j - FOG_OF_WAR_ORIGIN_X) / 2;
                int dy = i - FOG_OF_WAR_ORIGIN_Y;
                int dist_sq = (dx * dx) + (dy * dy);

                if (dist_sq > FOG_OF_WAR_FOG_RADIUS) {
                    ch = L' ';
                    color = COLOR_DEFAULT;
                } else if (dist_sq > FOG_OF_WAR_CLEAR_VISION_RADIUS) {
                    color = COLOR_GRAY;
                    ch = (ch == L' ') ? L' ' : L'░';
                }
            }

            if (color != current_color) {
                append_ansi_utf8(frame_buffer, &out_idx, ansi_from_color(color));
                current_color = color;
            }

            append_char_utf8(frame_buffer, &out_idx, ch);
        }
        frame_buffer[out_idx++] = '\n';
    }

    append_ansi_utf8(frame_buffer, &out_idx, ansi_from_color(COLOR_DEFAULT));
    frame_buffer[out_idx] = '\0';

    write(STDOUT_FILENO, frame_buffer, out_idx);
}
#endif
void set_glitch(int frame_nb, int chance) {
    glitch_ticks_left = max(0, frame_nb);
    glitch_chance = CLAMP(chance, 0, 100);
}
#pragma endregion