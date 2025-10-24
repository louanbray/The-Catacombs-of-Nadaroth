#include <pthread.h>
#include <string.h>

#include "achievements.h"
#include "assets_manager.h"
#include "audio_manager.h"
#include "entity.h"
#include "game_status.h"
#include "input_manager.h"
#include "logger.h"
#include "loot_manager.h"
#include "map.h"
#include "player.h"
#include "projectile.h"
#include "render.h"
#include "save_manager.h"
#include "statistics.h"

static int SEED;

/// @brief
/// Initialize global game state and core subsystems.
void init_game_system() {
    init_logger();  // Initialize logger first
    srand(SEED);
    init_terminal();
    init_assets_system();
    init_loot_tables();
    init_interactions_system();
    load_achievements();
    load_statistics();
    LOG_INFO("Game system initialized with SEED: %d", SEED);
}

/// @brief Handle the player movement and use the appropriate render
/// @param b board
/// @param p player
/// @param dir direction
void move(Render_Buffer* screen, player* p, int dir) {
    switch (move_player(p, dir)) {
        case MOVED_CHUNK:
            kill_all_projectiles(screen);
            render_from_player(screen, p);
            break;
        case CANT_MOVE:
            break;
        case PICKED_UP:
            render_player(screen, p);
            render_hotbar(screen, get_player_hotbar(p));
            break;
        case PICKED_UP_ENTITY:
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
int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-seed") == 0 && i + 1 < argc) {
            SEED = atoi(argv[i + 1]);
            break;
        } else
            SEED = time(NULL);
    }
    init_game_system();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-debug") == 0) {
            set_debug_mode(1);
            break;
        }
    }

    // if (init_audio() != 0) exit(EXIT_FAILURE);
    // play_bgm("assets/audio/background.mp3", 1);

    // ------------------- Create core game objects -------------------
    Render_Buffer* screen = create_screen();

    map* m = create_map();

    player* p = create_player(m);
    hotbar* h = create_hotbar();

    link_hotbar(p, h);

    init_projectile_system(screen, p, SEED);

    // ------------------- Start input processing thread -------------------
    pthread_t input_thread;
    InputThreadArgs input_args = {p, screen, fire_projectile, interact, arrow_move, compute_entry};

    if (pthread_create(&input_thread, NULL, process_input_thread, &input_args) != 0)
        return EXIT_FAILURE;

    pthread_detach(input_thread);

    // ------------------- Initial renders -------------------
    update_screen(screen);

    // ------------------- Show home menu and help -------------------
    home_menu(screen, p);
    display_interface(screen, "assets/interfaces/structures/help.dodjo");
    play_cinematic(screen, "assets/cinematics/oblivion.dodjo", 1000000);

    render(screen, m);
    update_screen(screen);

    increment_statistic(STAT_GAME_STARTED, 1);
    time_t start_time = time(NULL);

    // ------------------- Main game loop -------------------
    for (;;) {
        time_t current_time = time(NULL);
        if (GAME_PAUSED) start_time = current_time;
        int elapsed_seconds = (int)(current_time - start_time);
        if (elapsed_seconds != 0) {
            survivor_countdown(elapsed_seconds);
            increment_statistic(STAT_TIME_PLAYED, elapsed_seconds);
            start_time = current_time;
        }

        if (check_ctrl_c()) {
            LOG_INFO("CTRL+C detected, initiating graceful shutdown...");
            break;
        }

        if (USE_KEY('H') || USE_KEY('h')) {
            display_interface(screen, "assets/interfaces/structures/help.dodjo");
        } else if (USE_KEY('E') || USE_KEY('e')) {
            display_item_description(screen, get_selected_item(h));
        }

        if (is_debug_mode()) {
            if (USE_KEY('I') || USE_KEY('i')) {
                set_player_can_die(!can_player_die());
                LOG_INFO("Player can_die set to %d", can_player_die());
            }
            if (USE_KEY('P') || USE_KEY('p')) {
                if (GAME_PAUSED) {
                    resume_game();
                    unlock_inputs();
                } else {
                    pause_game();
                    lock_inputs();
                }
            }
            if (USE_KEY('R') || USE_KEY('r')) {
                kill_all_projectiles(screen);
            }
            if (USE_KEY('N') || USE_KEY('n')) {
                if (save_game("assets/data/save.dat", p, m, h)) {
                    LOG_INFO("Game saved successfully!");
                } else {
                    LOG_ERROR("Failed to save game");
                }
            }
            if (USE_KEY('B') || USE_KEY('b')) {
                if (load_game("assets/data/save.dat", p, m, h)) {
                    render(screen, m);
                    update_screen(screen);
                    LOG_INFO("Game loaded successfully!");
                } else {
                    LOG_ERROR("Failed to load game");
                }
            }
            if (USE_KEY('U') || USE_KEY('u')) {
                render(screen, m);
                update_screen(screen);
                LOG_INFO("Screen re-rendered");
            }
        }
    }

    // ------------------- Cleanup on exit -------------------
    LOG_INFO("Starting cleanup...");

    // Save game state
    save_achievements();
    save_statistics();
    LOG_INFO("Game state saved");

    // audio_close();
    destroy_interactions_system();

    LOG_INFO("Game session ended normally");
    close_logger();

    return EXIT_SUCCESS;
}