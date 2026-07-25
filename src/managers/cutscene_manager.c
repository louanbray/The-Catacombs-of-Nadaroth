#include "cutscene_manager.h"

#include "../display/render.h"
#include "../game_objects/map.h"
#include "../game_objects/player.h"
#include "../scripts/chunk_generation.h"
#include "../scripts/player_handler.h"
#include "../utils/game_status.h"
#include "../utils/logger.h"
#include "../utils/sys_platform.h"
#include "behaviour_manager.h"
#include "input_manager.h"
#include "projectile_manager.h"

typedef void (*cutscene_behaviour)(CutsceneID self_id, Render_Buffer* screen, map* m, player* p);

typedef struct PlayerOptions {
    Color color;
    PlayerClass class;
} PlayerOptions;

typedef struct MapOptions {
    ChunkType chunk_type;
    int spawn_x, spawn_y;
} MapOptions;

typedef struct Cutscene {
    PlayerOptions player_options;
    MapOptions map_options;
    cutscene_behaviour script;
} Cutscene;

#define DEFAULT_PLAYER_CLASS PLAYER_CLASS_BALL
#define DEFAULT_PLAYER_COLOR COLOR_YELLOW
#define DEFAULT_MAP_SPAWN CHUNK_SPAWN
#define DEFAULT_CHUNK_SPAWN_X 1
#define DEFAULT_CHUNK_SPAWN_Y 0

static Cutscene CUTSCENES[CUTSCENE_COUNT];
static bool CUTSCENE_SLOTS[CUTSCENE_COUNT] = {0};
static bool IN_CUTSCENE = false;

// ------------ Cutscene Engine ------------
static volatile CutsceneID PENDING_CUTSCENE = CUTSCENE_NONE;

static void add_cutscene(CutsceneID id, PlayerOptions player_o, MapOptions map_o, cutscene_behaviour script) {
    if (id <= CUTSCENE_NONE || id >= CUTSCENE_COUNT || CUTSCENE_SLOTS[id]) {
        LOG_ERROR("Couldn't link cutscene to id: %d", id);
        return;
    }

    CUTSCENES[id] = (Cutscene){
        .map_options = map_o,
        .player_options = player_o,
        .script = script,
    };
    CUTSCENE_SLOTS[id] = true;
}

void start_cutscene(CutsceneID id, Render_Buffer* screen, player* original_player) {
    if (id <= CUTSCENE_NONE || id >= CUTSCENE_COUNT || !CUTSCENE_SLOTS[id] || IN_CUTSCENE) {
        LOG_WARN("Couldn't start cutscene %d. Check if ID is out of range or if another cutscene is playing.", id);
        return;
    }
    IN_CUTSCENE = true;

    bool prev_gen = is_generation_enabled();
    bool prev_cache = is_cache_enabled();
    unsigned int pseed = get_projectile_seed();

    set_generation_enabled(false);
    set_enemy_random_cooldown_enabled(false);
    set_cache_enabled(false);

    pause_game();
    lock_inputs();

    backup_and_clear_projectiles();
    backup_and_clear_enemy_timers();

    Cutscene cscene = CUTSCENES[id];

    map* cmap = create_map_with_spawn(cscene.map_options.chunk_type, cscene.map_options.spawn_x, cscene.map_options.spawn_y);
    player* cplayer = cmap ? create_player(cmap) : NULL;
    hotbar* chotbar = cplayer ? create_hotbar() : NULL;

    if (!cmap || !cplayer || !chotbar) {
        LOG_ERROR("Failed to allocate cutscene resources for ID %d", id);
        goto cleanup;
    }

    link_hotbar(cplayer, chotbar);
    set_player_class(cplayer, cscene.player_options.class);
    set_player_color(cplayer, cscene.player_options.color);
    set_map_player(cmap, cplayer);
    set_player_map(cplayer, cmap);

    restart_projectile_system(screen, cplayer, 12345);

    render_from_player(screen, cplayer);
    update_screen(screen);

    if (cscene.script != NULL) cscene.script(id, screen, cmap, cplayer);

cleanup:
    kill_all_projectiles(screen);
    restart_projectile_system_restoring_enemy_timers(screen, original_player, 0);
    set_projectile_seed(pseed);
    restore_projectiles();

    if (chotbar) destroy_hotbar(chotbar);
    if (cplayer) destroy_player(cplayer);
    if (cmap) destroy_map(cmap);

    set_cache_enabled(prev_cache);
    set_generation_enabled(prev_gen);
    set_enemy_random_cooldown_enabled(true);
    resume_game();
    unlock_inputs();

    render_from_player(screen, original_player);
    IN_CUTSCENE = false;
}

bool is_in_cutscene() {
    return IN_CUTSCENE;
}

void request_cutscene(CutsceneID id) {
    PENDING_CUTSCENE = id;
}

void update_cutscenes(Render_Buffer* screen, player* main_player) {
    if (PENDING_CUTSCENE != CUTSCENE_NONE) {
        CutsceneID id_to_run = PENDING_CUTSCENE;
        PENDING_CUTSCENE = CUTSCENE_NONE;

        start_cutscene(id_to_run, screen, main_player);
    }
}

// --------- Cutscenes Scripts Utils ---------
void cutscene_wait(Render_Buffer* screen, int duration_ms) {
    int elapsed = 0;
    const int frame_time = 16;

    while (elapsed < duration_ms) {
        update_screen(screen);
        sys_sleep_ms(frame_time);
        elapsed += frame_time;
    }
}

// ------------ Cutscenes Scripts ------------
static void cutscene_script_test(CutsceneID id, Render_Buffer* screen, map* m, player* p) {
    (void)id;
    (void)m;
    for (int i = 0; i < 10; i++) {
        move(screen, p, DIR_NORTH);
        cutscene_wait(screen, 60);
    }
    for (int i = 0; i < 10; i++) {
        move(screen, p, DIR_EAST);
        cutscene_wait(screen, 60);
    }
    for (int i = 0; i < 10; i++) {
        move(screen, p, DIR_SOUTH);
        cutscene_wait(screen, 60);
    }
    for (int i = 0; i < 10; i++) {
        move(screen, p, DIR_WEST);
        cutscene_wait(screen, 60);
    }
}

// ------------------ Setup ------------------
void init_cutscenes() {
    add_cutscene(
        CUTSCENE_TEST,
        (PlayerOptions){
            .class = PLAYER_CLASS_BALL,
            .color = COLOR_MAGENTA_BOLD,
        },
        (MapOptions){
            .chunk_type = CHUNK_RANDOM_EASY,
            .spawn_x = DEFAULT_CHUNK_SPAWN_X,
            .spawn_y = DEFAULT_CHUNK_SPAWN_Y,
        },
        cutscene_script_test);
}