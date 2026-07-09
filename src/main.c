#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "display/render.h"
#include "game_objects/entity.h"
#include "game_objects/map.h"
#include "game_objects/player.h"
#include "managers/achievements_manager.h"
#include "managers/assets_manager.h"
#include "managers/audio_manager.h"
#include "managers/input_manager.h"
#include "managers/loot_manager.h"
#include "managers/projectile_manager.h"
#include "managers/save_manager.h"
#include "managers/settings_manager.h"
#include "managers/statistics_manager.h"
#include "scripts/player_handler.h"
#include "utils/game_status.h"
#include "utils/logger.h"

static unsigned int SEED;
static map* MAP_L;
static player* PLAYER_L;
static hotbar* HOTBAR_L;

#include <errno.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

bool directory_exists(const char* path) {
    struct stat st;

    if (stat(path, &st) != 0)
        return false;

#ifdef _WIN32
    return (st.st_mode & _S_IFDIR) != 0;
#else
    return S_ISDIR(st.st_mode);
#endif
}

bool create_dir_if_not_exists(const char* path) {
    if (MKDIR(path) == 0)
        return true;

    if (errno == EEXIST)
        return directory_exists(path);

    return false;
}

/// @brief
/// Initialize global game state and core subsystems.
void init_game_system() {
    init_logger();  // Initialize logger first
    srand(SEED);
    init_terminal();
    init_assets_system();
    init_loot_tables();
    seed_loot_manager(SEED);
    init_interactions_system();
    load_achievements();
    load_statistics();
    load_settings();
    LOG_INFO("Game system initialized with SEED: %d", SEED);
}

/// @brief Handle the player movement and use the appropriate render
/// @param b board
/// @param p player
/// @param dir direction
void move(Render_Buffer* screen, player* p, int dir) {
    switch (move_player(p, dir)) {
        case MOV_MOVED_CHUNK:
            kill_all_projectiles(screen);
            render_from_player(screen, p);
            break;
        case MOV_CANT_MOVE:
            break;
        case MOV_PICKED_UP:
            render_player(screen, p);
            render_hotbar(screen, get_player_hotbar(p));
            render_keyholder(screen, get_player_keyholder(p));
            break;
        case MOV_PICKED_UP_ENTITY:
            render_from_player(screen, p);
            break;
        default:
            render_player(screen, p);
            break;
    }
}

#ifdef _WIN32
static const unsigned long long HOLD_REPEAT_DELAY_MS = 60ULL;
static const unsigned long long HOLD_REPEAT_INTERVAL_MS = 60ULL;

static bool is_virtual_key_down(int virtual_key) {
    return (GetAsyncKeyState(virtual_key) & 0x8000) != 0;
}

enum HeldMoveMask {
    MOVE_NONE = 0,
    MOVE_NORTH = 1 << 0,
    MOVE_SOUTH = 1 << 1,
    MOVE_WEST = 1 << 2,
    MOVE_EAST = 1 << 3,
};

static unsigned int get_held_move_mask() {
    bool up = is_virtual_key_down('Z');
    bool down = is_virtual_key_down('S');
    bool left = is_virtual_key_down('Q');
    bool right = is_virtual_key_down('D');

    if (up && down) {
        up = false;
        down = false;
    }
    if (left && right) {
        left = false;
        right = false;
    }

    return (up ? MOVE_NORTH : MOVE_NONE) | (down ? MOVE_SOUTH : MOVE_NONE) |
           (left ? MOVE_WEST : MOVE_NONE) | (right ? MOVE_EAST : MOVE_NONE);
}

static bool apply_move_mask(Render_Buffer* screen, player* p, unsigned int mask) {
    bool moved = false;

    if (mask & MOVE_NORTH) {
        move(screen, p, DIR_NORTH);
        moved = true;
    }
    if (mask & MOVE_SOUTH) {
        move(screen, p, DIR_SOUTH);
        moved = true;
    }
    if (mask & MOVE_WEST) {
        move(screen, p, DIR_WEST);
        moved = true;
    }
    if (mask & MOVE_EAST) {
        move(screen, p, DIR_EAST);
        moved = true;
    }

    return moved;
}

static void process_held_movement(Render_Buffer* screen, player* p) {
    static unsigned int last_mask = MOVE_NONE;
    static unsigned long long next_move_time = 0;
    static unsigned long long repeat_start_time = 0;

    unsigned int mask = get_held_move_mask();
    unsigned long long now = GetTickCount64();

    if (mask == MOVE_NONE) {
        last_mask = MOVE_NONE;
        next_move_time = 0;
        repeat_start_time = 0;
        return;
    }

    if (mask != last_mask) {
        if (apply_move_mask(screen, p, mask)) {
            update_screen(screen);
        }
        last_mask = mask;
        repeat_start_time = now + HOLD_REPEAT_DELAY_MS;
        next_move_time = repeat_start_time;
        return;
    }

    if (now < repeat_start_time) {
        return;
    }

    if (now >= next_move_time) {
        if (apply_move_mask(screen, p, mask)) {
            update_screen(screen);
        }
        next_move_time += HOLD_REPEAT_INTERVAL_MS;
        if (next_move_time < now) {
            next_move_time = now + HOLD_REPEAT_INTERVAL_MS;
        }
    }
}
#endif

/// @brief Handle the user keyboard entries
/// @param entry entry
/// @param screen Render_Buffer
/// @param p player
void compute_entry(Render_Buffer* screen, player* p, int entry) {
    hotbar* hb = get_player_hotbar(p);
    switch (entry) {
#ifdef _WIN32
        case KEY_Z_LOW:
        case KEY_Z_HIGH:
        case KEY_S_LOW:
        case KEY_S_HIGH:
        case KEY_Q_LOW:
        case KEY_Q_HIGH:
        case KEY_D_LOW:
        case KEY_D_HIGH:
            return;
#else
        case KEY_Z_LOW:
        case KEY_Z_HIGH:
            move(screen, p, DIR_NORTH);
            break;
        case KEY_S_LOW:
        case KEY_S_HIGH:
            move(screen, p, DIR_SOUTH);
            break;
        case KEY_Q_LOW:
        case KEY_Q_HIGH:
            move(screen, p, DIR_WEST);
            break;
        case KEY_D_LOW:
        case KEY_D_HIGH:
            move(screen, p, DIR_EAST);
            break;
#endif

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
            drop(p, get_selected_slot(hb));
            render_hotbar(screen, hb);
            render_chunk(screen, get_player_chunk(p));
            break;

        default:
            return;
    }

    update_screen(screen);
}

void interact(Render_Buffer* screen, player* p, int x, int y) {
    hotbar* hb = get_player_hotbar(p);
    bool chest_opened = check_lootable_interaction(p, x, y);
    bool destroy = false;

    item* it = get_selected_item(hb);
    if (it != NULL) {
        UsableItem type = get_item_usable_type(it);
        if (type == USABLE_ITEM_NOT_USABLE) return;

        switch (type) {
            case USABLE_ITEM_GOLDEN_APPLE:
                destroy = true;
                set_player_max_health(p, get_player_max_health(p) + 1);
                break;
            case USABLE_ITEM_ONION_RING:
                if (get_player_max_health(p) != get_player_health(p)) {
                    destroy = true;
                    heal_player(p, 1);
                }
                break;
            case USABLE_ITEM_STOCKFISH:
                if (get_player_max_health(p) != get_player_health(p)) {
                    destroy = true;
                    heal_player(p, get_player_max_health(p) - get_player_health(p));
                }
                break;
            case USABLE_ITEM_BOMB:
                destroy = true;
                destroy_player_cchunk(p);
                break;
            case USABLE_ITEM_SCHOOL_DISHES:
                if (get_player_mental_health(p) != 4) {
                    destroy = true;
                    modify_player_mental_health(p, 1);
                }
                break;
            case USABLE_ITEM_FORGOTTEN_DISH:
                if (get_player_mental_health(p) != 4) {
                    destroy = true;
                    set_player_mental_health(p, 4);
                }
                break;
            default:
                if (chest_opened) break;
                return;
        }
    }

    if (chest_opened) {
        render_from_player(screen, p);
    } else if (destroy) {
        hotbar_drop(hb, get_selected_slot(hb), true);
        render_health(screen, p);
        render_hotbar(screen, hb);
        render_score(screen, p);
        render_mental_health(screen, p);
    }

    update_screen(screen);
}

void scroll_callback(Render_Buffer* screen, player* p, int x, int y, int direction) {
    (void)x;
    (void)y;
    hotbar* hb = get_player_hotbar(p);
    int selected = get_selected_slot(hb);

    if (direction > 0) {
        // Scroll up
        selected--;
        if (selected < 0) selected = HOTBAR_SIZE - 1;
    } else {
        // Scroll down
        selected++;
        if (selected >= HOTBAR_SIZE) selected = 0;
    }

    select_slot(hb, selected);
    render_hotbar(screen, hb);
    update_screen(screen);
}

void* process_input_thread(void* arg) {
    InputThreadArgs* args = (InputThreadArgs*)arg;
    process_input(args->p, args->screen, args->mouse_left_event_callback, args->mouse_right_event_callback, args->mouse_scroll_callback, args->printable_char_callback);
    return NULL;
}

static void init_local_elements() {
    MAP_L = create_map();
    PLAYER_L = create_player(MAP_L);
    HOTBAR_L = create_hotbar();
    link_hotbar(PLAYER_L, HOTBAR_L);
}

static void clear_local_elements() {
    destroy_hotbar(HOTBAR_L);
    destroy_player(PLAYER_L);
    destroy_map(MAP_L);
}

void handle_resume(ResumeState state, Render_Buffer* screen) {
    if (state == RESUME_DEFAULT) return;
    if (state == RESUME_NEW_GAME) {
        SEED = time(NULL);
        increment_statistic(STAT_GAME_STARTED, 1);
    }

    Color player_color = get_player_color(PLAYER_L);
    PlayerClass player_class = get_player_class(PLAYER_L);

    srand(SEED);
    seed_loot_manager(SEED);
    lock_inputs();
    pause_game();

    reset_run_based_achievements();

    kill_all_projectiles(screen);
    clear_screen(get_board(screen));
    stop_projectile_system();

    clear_local_elements();
    init_local_elements();

    init_projectile_system(screen, PLAYER_L, SEED);

    if ((state == RESUME_RESET && get_setting_value(SETTING_SKIP_PLAYER_CUSTOM_ON_RESET)) || (state == RESUME_NEW_GAME && get_setting_value(SETTING_SKIP_PLAYER_CUSTOM_ON_NEW_GAME))) {
        set_player_color(PLAYER_L, player_color);
        set_player_class(PLAYER_L, player_class);
    }

    home_menu(screen, PLAYER_L, state);
    set_time_played((struct timeval){0, 0});

    render_from_player(screen, PLAYER_L);
    update_screen(screen);

    unlock_inputs();
    resume_game();
}

/// @brief Where it all begins
/// @return I dream of a 0
int main(int argc, char* argv[]) {
    create_dir_if_not_exists("data");
    SEED = (unsigned int)time(NULL);
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-seed") == 0 && i + 1 < argc) {
            SEED = (unsigned int)atoi(argv[i + 1]);
            break;
        }
    }
    init_game_system();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-debug") == 0) {
            set_debug_mode(1);
            break;
        }
    }

    if (init_audio() != 0) exit(EXIT_FAILURE);
    play_bgm("assets/audio/background.mp3", 1);

    // ------------------- Create core game objects -------------------
    Render_Buffer* screen = create_screen();

    init_local_elements();

    init_projectile_system(screen, PLAYER_L, SEED);

    // ------------------- Start input processing thread -------------------
    pthread_t input_thread;
    InputThreadArgs input_args = {&PLAYER_L, screen, fire_projectile, interact, scroll_callback, compute_entry};

    if (pthread_create(&input_thread, NULL, process_input_thread, &input_args) != 0)
        return EXIT_FAILURE;

    // ------------------- Initial renders -------------------
    update_screen(screen);

    // ------------------- Show home menu and help -------------------
    home_menu(screen, PLAYER_L, RESUME_DEFAULT);

    if (!get_setting_value(SETTING_SKIP_INTRO)) {
        display_interface(screen, "assets/interfaces/structures/help.dodjo");
        play_cinematic(screen, "assets/cinematics/oblivion.dodjo", CINEMATIC_FRAME_DELAY);
    }

    render(screen, MAP_L);
    update_screen(screen);

    increment_statistic(STAT_GAME_STARTED, 1);

    struct timeval start_time, current_time;
    gettimeofday(&start_time, NULL);
    double accumulated_time = 0.0;

    // ------------------- Main game loop -------------------
    for (;;) {
        gettimeofday(&current_time, NULL);

        if (GAME_PAUSED || need_reset())
            start_time = current_time;
        else {
            double elapsed = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_usec - start_time.tv_usec) / 1000000.0;
            add_time_played((struct timeval){.tv_sec = (long)elapsed, .tv_usec = (long)((elapsed - (long)elapsed) * 1000000)});
            accumulated_time += elapsed;
            start_time = current_time;

            if (accumulated_time >= 1.0) {
                int full_seconds = (int)accumulated_time;
                survivor_countdown(PLAYER_L, full_seconds);
                increment_statistic(STAT_TIME_PLAYED, full_seconds);
                render_timer(screen);
                accumulated_time -= full_seconds;
            }
        }

        if (!is_game_running()) {
            LOG_INFO("CTRL+C detected, initiating graceful shutdown...");
            pause_game();
            lock_inputs();
            kill_all_projectiles(screen);
            break;
        }

        if (is_debug_mode() && (USE_KEY('P') || USE_KEY('p'))) {
            if (GAME_PAUSED) {
                resume_game();
            } else {
                pause_game();
            }
        }

#ifdef _WIN32
        if (!GAME_PAUSED || is_debug_mode()) {
            process_held_movement(screen, PLAYER_L);
        }
#endif

        if (GAME_PAUSED) {
            struct timespec ts = {.tv_sec = 0, .tv_nsec = 1000000};
            nanosleep(&ts, NULL);
            continue;
        }

        if (USE_KEY('E') || USE_KEY('e')) {
            display_item_description(screen, get_selected_item(HOTBAR_L));
        } else if (USE_KEY(' ')) {
            handle_resume(pause_menu(screen, PLAYER_L, MAP_L, HOTBAR_L), screen);
        } else if (USE_KEY('H') || USE_KEY('h')) {
            display_interface(screen, "assets/interfaces/structures/help.dodjo");
        } else if (USE_KEY('A') || USE_KEY('a')) {
            display_achievements(screen, 0);
        } else if (USE_KEY('T') || USE_KEY('t')) {
            display_statistics(screen);
        }

        if (is_debug_mode()) {
            if (USE_KEY('I') || USE_KEY('i')) {
                set_player_can_die(!can_player_die());
                LOG_INFO("Player can_die set to %d", can_player_die());
                if (!can_player_die())
                    disable_sound_effect(AUDIO_PLAYER_HURT);
                else
                    enable_sound_effect(AUDIO_PLAYER_HURT);
            } else if (USE_KEY('R') || USE_KEY('r')) {
                kill_all_projectiles(screen);
            } else if (USE_KEY('U') || USE_KEY('u')) {
                render(screen, MAP_L);
                update_screen(screen);
                LOG_INFO("Screen re-rendered");
            } else if (USE_KEY('M') || USE_KEY('m')) {
                modify_player_mental_health(PLAYER_L, 1);
                render_mental_health(screen, PLAYER_L);
                update_screen(screen);
                LOG_INFO("Player mental health increased by 1");
            } else if (USE_KEY('L') || USE_KEY('l')) {
                modify_player_mental_health(PLAYER_L, -1);
                render_mental_health(screen, PLAYER_L);
                update_screen(screen);
                LOG_INFO("Player mental health decreased by 1");
            } else if (USE_KEY('<')) {
                keyholder* k = get_player_keyholder(PLAYER_L);
                set_keyholder_level(k, get_keyholder_level(k) - 1);
                render_keyholder(screen, k);
                update_screen(screen);
                LOG_INFO("Player keyholder level decreased by 1");
            } else if (USE_KEY('>')) {
                keyholder* k = get_player_keyholder(PLAYER_L);
                keyholder_level_up(k);
                render_keyholder(screen, k);
                update_screen(screen);
                LOG_INFO("Player keyholder level increased by 1");
            } else if (USE_KEY(';')) {
                keyholder* k = get_player_keyholder(PLAYER_L);
                add_keyholder_keys_of_rarity(k, RARITY_BRONZE, -1);
                render_keyholder(screen, k);
                update_screen(screen);
                LOG_INFO("Player keyholder level decreased by 1");
            } else if (USE_KEY(':')) {
                keyholder* k = get_player_keyholder(PLAYER_L);
                add_keyholder_keys_of_rarity(k, RARITY_BRONZE, 1);
                render_keyholder(screen, k);
                update_screen(screen);
                LOG_INFO("Player keyholder level increased by 1");
            } else if (USE_KEY('K') || USE_KEY('k')) {
                simulate_projectile_hit(get_player_health(PLAYER_L), PLAYER_L, screen);
                render_health(screen, PLAYER_L);
                update_screen(screen);
                LOG_INFO("Player damaged to death");
            } else if (USE_KEY('C') || USE_KEY('c')) {
                heal_player(PLAYER_L, get_player_max_health(PLAYER_L));
                render_health(screen, PLAYER_L);
                update_screen(screen);
                LOG_INFO("Player health restored to max");
            } else if (USE_KEY('F') || USE_KEY('f')) {
                set_player_score(PLAYER_L, ScorePerPhase[get_player_phase(PLAYER_L)]);
                render_score(screen, PLAYER_L);
                update_screen(screen);
                LOG_INFO("Player score set to phase score");
            }
        }

        // Cap the loop to ~1000 iterations/sec to avoid burning a CPU core
        struct timespec ts = {.tv_sec = 0, .tv_nsec = 1000000};
        nanosleep(&ts, NULL);
    }

    // ------------------- Stop input thread -------------------
    pthread_cancel(input_thread);
    pthread_join(input_thread, NULL);
    LOG_INFO("Input thread stopped");

    // ------------------- Cleanup on exit -------------------
    LOG_INFO("Starting cleanup...");

    stop_projectile_system();
    restore_terminal_mode();

    // Save game state
    save_achievements();
    save_statistics();
    LOG_INFO("Game state saved");

    audio_close();
    destroy_interactions_system();
    destroy_asset_manager();
    clear_local_elements();

    LOG_INFO("Game session ended normally");
    close_logger();

    return EXIT_SUCCESS;
}