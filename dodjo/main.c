#include "entity.h"
#include "input_manager.h"
#include "map.h"
#include "player.h"
#include "projectile.h"
#include "render.h"

#define CHAR_TO_INT 49

/// @brief Handle the player movement and use the appropriate render
/// @param b board
/// @param p player
/// @param dir direction
void move(Render_Buffer* screen, player* p, int dir) {
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

/// @brief Handle the player movement with the arrow keys
/// @param screen screen
/// @param p player
/// @param key_code keycode
void arrow_move(Render_Buffer* screen, player* p, int key_code) {
    switch (key_code) {
        case KEY_ARROW_UP:
            move(screen, p, NORTH);
            break;
        case KEY_ARROW_DOWN:
            move(screen, p, SOUTH);
            break;
        case KEY_ARROW_RIGHT:
            move(screen, p, EAST);
            break;
        case KEY_ARROW_LEFT:
            move(screen, p, WEST);
            break;
        default:
            return;
    }
    update_screen(screen);
}

/// @brief Handle the user keyboard entries
/// @param entry entry
/// @param screen Render_Buffer
/// @param p player
void compute_entry(Render_Buffer* screen, player* p, int entry) {
    switch (entry) {
        case KEY_Z_LOW:
        case KEY_Z_HIGH:
            move(screen, p, NORTH);
            break;
        case KEY_S_LOW:
        case KEY_S_HIGH:
            move(screen, p, SOUTH);
            break;
        case KEY_Q_LOW:
        case KEY_Q_HIGH:
            move(screen, p, WEST);
            break;
        case KEY_D_LOW:
        case KEY_D_HIGH:
            move(screen, p, EAST);
            break;

        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
        case KEY_6:
        case KEY_7:
        case KEY_8:
        case KEY_9:
            select_slot(get_player_hotbar(p), entry - CHAR_TO_INT);
            render_hotbar(screen, get_player_hotbar(p));
            break;

        case KEY_W_LOW:
        case KEY_W_HIGH:
            drop(get_player_hotbar(p), get_selected_slot(get_player_hotbar(p)));
            render_hotbar(screen, get_player_hotbar(p));
            break;

        default:
            return;
    }

    update_screen(screen);
}

/// @brief Where it all begins
/// @return I dream of a 0
int main() {
    init_terminal();

    Render_Buffer* screen = create_screen();

    map* m = create_map();

    player* p = create_player(m);
    hotbar* h = create_hotbar();

    link_hotbar(p, h);

    render(screen, m);
    update_screen(screen);

    init_projectile_system(screen);
    process_input(p, screen, fire_projectile, arrow_move, compute_entry);

    return EXIT_SUCCESS;
}