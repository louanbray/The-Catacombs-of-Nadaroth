#include <pthread.h>

#include "assets_manager.h"
#include "audio_manager.h"
#include "entity.h"
#include "game_status.h"
#include "input_manager.h"
#include "loot_manager.h"
#include "map.h"
#include "player.h"
#include "projectile.h"
#include "render.h"

/// @brief Handle the player movement and use the appropriate render
/// @param b board
/// @param p player
/// @param dir direction
void move(Render_Buffer* screen, player* p, int dir) {
    switch (move_player(p, dir)) {
        case 1:
            kill_all_projectiles(screen);
            render_from_player(screen, p);
            break;
        case 2:
            break;
        case 3:
            render_player(screen, p);
            render_hotbar(screen, get_player_hotbar(p));
            break;
        case 4:
            render_from_player(screen, p);
            render_hotbar(screen, get_player_hotbar(p));
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
    hotbar* hb = get_player_hotbar(p);
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
            select_slot(hb, entry - CHAR_TO_INT);
            render_hotbar(screen, hb);
            break;

        case KEY_W_LOW:
        case KEY_W_HIGH:
            drop(hb, get_selected_slot(hb));
            render_hotbar(screen, hb);
            break;

        default:
            return;
    }

    update_screen(screen);
}

void interact(Render_Buffer* screen, player* p) {
    hotbar* hb = get_player_hotbar(p);
    item* it = get_selected_item(hb);
    if (it == NULL) return;

    UsableItem type = get_item_usable_type(it);
    if (type == NOT_USABLE_ITEM) return;

    bool destroy = false;

    switch (type) {
        case GOLDEN_APPLE:
            destroy = true;
            set_player_max_health(p, get_player_max_health(p) + 1);
            break;
        case ONION_RING:
            if (get_player_max_health(p) != get_player_health(p)) {
                destroy = true;
                heal_player(p, 1);
            }
            break;
        case STOCKFISH:
            if (get_player_max_health(p) != get_player_health(p)) {
                destroy = true;
                heal_player(p, get_player_max_health(p) - get_player_health(p));
            }
            break;
        case BOMB:
            destroy = true;
            destroy_player_cchunk(p);
            break;
        case SCHOOL_DISHES:
            if (get_player_mental_health(p) != 4) {
                destroy = true;
                modify_player_mental_health(p, 1);
            }
            break;
        case FORGOTTEN_DISH:
            if (get_player_mental_health(p) != 4) {
                destroy = true;
                set_player_mental_health(p, 4);
            }
            break;
        default:
            return;
    }

    if (destroy) {
        drop(hb, get_selected_slot(hb));
        render_health(screen, p);
        render_hotbar(screen, hb);
        render_score(screen, p);
        render_mental_health(screen, p);
    }

    update_screen(screen);
}

typedef struct {
    player* p;
    Render_Buffer* screen;
    void (*mouse_left_event_callback)(Render_Buffer* screen, player* p, int x, int y);
    void (*mouse_right_event_callback)(Render_Buffer* screen, player* p);
    void (*arrow_key_callback)(Render_Buffer* screen, player* p, int arrow_key);
    void (*printable_char_callback)(Render_Buffer* screen, player* p, int c);
} InputThreadArgs;

void* process_input_thread(void* arg) {
    InputThreadArgs* args = (InputThreadArgs*)arg;
    process_input(args->p, args->screen, args->mouse_left_event_callback, args->mouse_right_event_callback, args->arrow_key_callback, args->printable_char_callback);
    return NULL;
}

/// @brief Where it all begins
/// @return I dream of a 0
int main() {
    init_terminal();
    init_assets_system();
    init_loot_tables();

    // if (init_audio() != 0) exit(EXIT_FAILURE);

    // play_bgm("assets/audio/background.mp3", 1);

    Render_Buffer* screen = create_screen();

    map* m = create_map();

    player* p = create_player(m);
    hotbar* h = create_hotbar();

    link_hotbar(p, h);

    init_projectile_system(screen, p);

    pthread_t input_thread;
    InputThreadArgs input_args = {p, screen, fire_projectile, interact, arrow_move, compute_entry};

    if (pthread_create(&input_thread, NULL, process_input_thread, &input_args) != 0)
        return EXIT_FAILURE;

    pthread_detach(input_thread);

    update_screen(screen);
    display_interface(screen, "assets/interfaces/structures/start_menu.dodjo");
    display_interface(screen, "assets/interfaces/structures/help.dodjo");
    play_cinematic(screen, "assets/cinematics/oblivion.dodjo", 1000000);

    render(screen, m);
    update_screen(screen);

    for (;;) {
        if (GAME_PAUSED) {
            if (USE_KEY('P') || USE_KEY('p')) {
                resume_game();
                unlock_inputs();
            }
            continue;
        } else if (USE_KEY('H') || USE_KEY('h')) {
            display_interface(screen, "assets/interfaces/structures/help.dodjo");
        } else if (USE_KEY('E') || USE_KEY('e')) {
            display_item_description(screen, get_selected_item(h));
        } else if (USE_KEY('R') || USE_KEY('r')) {
            kill_all_projectiles(screen);
        }
    }

    // audio_close();

    return EXIT_SUCCESS;
}