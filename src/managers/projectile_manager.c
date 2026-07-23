#include "projectile_manager.h"

#include <limits.h>

#include "../display/render.h"
#include "../game_objects/entity.h"
#include "../game_objects/player.h"
#include "../utils/constants.h"
#include "../utils/game_status.h"
#include "../utils/logger.h"
#include "../utils/sys_platform.h"
#include "achievements_manager.h"
#include "audio_manager.h"
#include "behaviour_manager.h"
#include "input_manager.h"
#include "loot_manager.h"
#include "statistics_manager.h"

#define MAX_PROJECTILES 128

#define FIRING_RANDOM_OFFSET_MAX 30

#define EASY_MODE_INV_FRAMES 180
#define HARD_MODE_INV_FRAMES 6

#define NADINO_ENEMY_MIN_SCORE 71

#define XSPACING 2
//? As the projectiles uses x/2, used to work with odd coordinates
#define PADDING_X ((GAME_PADDING / 2) % 2)

static unsigned int projectile_rng_seed = 0;
static int total_enemies = 0;
static int total_player_projectiles = 0;
static int tracked_enemy_count = -1;

typedef struct {
    Render_Buffer* r;
    player* p;
} InitThreadArgs;

typedef struct Projectile {
    int x, y;                        // Current position
    int x1, y1;                      // Target position
    int dx, dy;                      // Absolute differences
    int sx, sy;                      // Step directions
    int err;                         // Error term
    unsigned int from;               // Character to ignore
    int from_id;                     // Character to ignore
    int frame, rate;                 // Animation frame and rate
    int range;                       // Maximum range (-1 for unlimited)
    int distance_traveled;           // Distance traveled so far
    unsigned int design;             // Design character
    bool infinity;                   // Is the projectile infinite?
    bool home;                       // Is the projectile homing?
    bool active;                     // Is the projectile active?
    ProjectileCallback callback;     // Callback to execute on hit
    projectile_data* callback_data;  // Additional data for callback
} Projectile;

Projectile projectiles[MAX_PROJECTILES];

pthread_mutex_t projectile_mutex;

static pthread_t projectile_thread;
static bool projectile_thread_running = false;
static bool projectile_thread_stop = false;
static bool projectile_mutex_ready = false;

pthread_mutex_t entity_mutex = PTHREAD_MUTEX_INITIALIZER;

void kill_all_projectiles(Render_Buffer* r) {
    pthread_mutex_lock(&projectile_mutex);
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        Projectile* p = &projectiles[i];
        if (p->active) {
            unsigned int c = render_get_cell_char(r, ITR(p->y), ITR(p->x * XSPACING + PADDING_X));

            if (c == L' ' || c == p->design) {
                render_set_cell_char(r, ITR(p->y), ITR(p->x * XSPACING + PADDING_X), L' ');
            }
        }
        if (p->callback_data) {
            free(p->callback_data);
            p->callback_data = NULL;
        }
        p->active = false;
    }
    pthread_mutex_unlock(&projectile_mutex);
}

static void draw_ppos(Render_Buffer* r, Projectile* p, unsigned int character) {
    render_set_cell_char(r, ITR(p->y), ITR(p->x * XSPACING + PADDING_X), character);
}

static void draw_ppos_if_nothing_here(Render_Buffer* r, Projectile* p, unsigned int character) {
    unsigned int c = render_get_cell_char(r, ITR(p->y), ITR(p->x * XSPACING + PADDING_X));
    bool nothing = c == L' ' || c == p->design;
    if (nothing) render_set_cell_char(r, ITR(p->y), ITR(p->x * XSPACING + PADDING_X), character);
}

// Bresenham's Line Algorithm
void update_projectiles(Render_Buffer* r) {
    pthread_mutex_lock(&projectile_mutex);
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!projectiles[i].active) continue;

        Projectile* p = &projectiles[i];

        p->frame++;
        if (p->frame != p->rate) continue;
        p->frame = 0;

        unsigned int c = render_get_cell_char(r, ITR(p->y), ITR(p->x * XSPACING + PADDING_X));
        bool nothing = c == L' ' || c == p->design;

        if (nothing) draw_ppos(r, p, L' ');

        // Check if range limit is exceeded (only if range >= 0)
        bool range_exceeded = false;
        if (p->range >= 0 && p->distance_traveled >= p->range) {
            range_exceeded = true;
        }

        if ((c != p->from || !p->home) && (((p->x == p->x1 && p->y == p->y1) && !p->infinity) || !nothing || range_exceeded)) {
            p->active = false;
            if (p->design == PLAYER_PROJECTILE_DESIGN) {
                total_player_projectiles--;
                total_player_projectiles = total_player_projectiles < 0 ? 0 : total_player_projectiles;
            }
            if (p->callback) {
                void* data_ptr = p->callback_data;
                p->callback_data = NULL;
                p->callback(p->x * XSPACING + PADDING_X - RECENTER_X, RECENTER_Y - p->y, data_ptr);
            }

            continue;
        }

        int e2 = XSPACING * p->err;
        if (e2 >= p->dy) {
            p->err += p->dy;
            p->x += p->sx;
            p->distance_traveled++;  // Increment distance when moving horizontally
        }
        if (e2 <= p->dx) {
            p->err += p->dx;
            p->y += p->sy;
            p->distance_traveled++;  // Increment distance when moving vertically
        }

        c = render_get_cell_char(r, ITR(p->y), ITR(p->x * XSPACING + PADDING_X));
        if (c == L' ' || c == p->design) {
            if (p->home) p->home = false;
            draw_ppos(r, p, p->design);
        }
    }

    pthread_mutex_unlock(&projectile_mutex);
    fflush(stdout);
}

// Function to animate a projectile moving from (x0, y0) to (x1, y1)
void projectile_callback(int x, int y, projectile_data* data) {
    chunk* current_ck = get_player_chunk(data->p);
    if (chunk_has_wall(current_ck, x, y)) {
        free(data);
        return;
    }

    item* it = get_hm(get_chunk_furniture_coords(current_ck), x, y);

    if (it == NULL) {
        free(data);
        return;
    }

    bool is_entity = is_an_entity(it);
    entity* ent = NULL;

    if (is_entity) {
        ent = get_entity_link(it);
        it = get_entity_brain(ent);
    }

    switch (get_item_type(it)) {
        case ITEMTYPE_ENEMY: {
            enemy* e = get_item_spec(it);

            pthread_mutex_lock(&entity_mutex);

            e->hp -= data->damage;

            if (e->hp <= 0) {
                increment_statistic(STAT_ENEMIES_KILLED, 1);
                add_achievement_progress(ACH_UNSTOPPABLE, 1);
                set_achievement_progress(ACH_MONSTER_HUNTER, 1);
                if (e->score >= NADINO_ENEMY_MIN_SCORE) add_achievement_progress(ACH_BOSS_SLAYER, 1);

                if (e->can_drop) {
                    item* drop = generate_loot(&e->loot);
                    if (drop) {
                        set_item_x(drop, get_item_x(it));
                        set_item_y(drop, get_item_y(it));
                        add_item(get_player_chunk(data->p), drop);
                    }
                }

                add_player_score(data->p, e->score);
                set_dyn(get_chunk_enemies(get_player_chunk(data->p)), e->from_id, NULL);
                total_enemies--;
                if (is_entity) {
                    destroy_entity_from_chunk(ent);
                    render_from_player(data->screen, data->p);
                } /*else {
                    remove_item(get_player_chunk(data->p), it);
                    render_char(get_board(data->screen), x, y, ' ');
                }*/
                play_sound_effect_by_id(AUDIO_ENEMY_KILLED);
            }

            pthread_mutex_unlock(&entity_mutex);

            break;
        }

        default:
            break;
    }
    free(data);
}

void enemy_attack_callback(int x, int y, projectile_data* data) {
    bool dead = false;
    if (x == get_player_x(data->p) && y == get_player_y(data->p)) {
        play_sound_effect_by_id(AUDIO_PLAYER_HURT);
        if (damage_player(data->p, data->damage)) {
            player_death(data->p);
            render_from_player(data->screen, data->p);
            dead = true;
        } else
            render_health(data->screen, data->p);
    }
    if (dead) {
        kill_all_projectiles(data->screen);
        char filepath[PATH_MAX];
        GamePhase phase = get_player_phase(data->p);
        snprintf(filepath, sizeof(filepath), "assets/cinematics/lore/%d/%d.dodjo", get_player_mental_health(data->p), phase);
        if (phase == GAMEPHASE_FIRST_ACT_END) {
            LOG_INFO("Game completed in %ld seconds and %ld microseconds", get_time_played().tv_sec, get_time_played().tv_usec);
            increment_statistic(STAT_GAME_COMPLETIONS, 1);
            set_achievement_progress(ACH_DAWN_BREAKER, 1);
            if (get_player_mental_health(data->p) == MAX_MENTAL_HEALTH) {
                set_achievement_progress(ACH_UNSHAKEN, 1);
                if (get_player_design(data->p) == PLAYER_DESIGN_BALL)
                    increment_statistic(STAT_GAME_COMPLETION_AS_BALL, 1);
                else if (get_player_design(data->p) == PLAYER_DESIGN_CAMO)
                    increment_statistic(STAT_GAME_COMPLETION_AS_CAMO, 1);
                else if (get_player_design(data->p) == PLAYER_DESIGN_BRAWLER)
                    increment_statistic(STAT_GAME_COMPLETION_AS_BRAWLER, 1);
                else if (get_player_design(data->p) == PLAYER_DESIGN_SHIELD)
                    increment_statistic(STAT_GAME_COMPLETION_AS_SHIELD, 1);
                if (get_time_played().tv_sec < 600) {  // 10mn
                    increment_statistic(STAT_SPEED_RUNS, 1);
                    set_achievement_progress(ACH_SPEED_RUNNER, 1);
                }
                if (total_enemies == 0) {
                    set_achievement_progress(ACH_PERFECTIONIST, 1);
                }
            }
            if (get_statistic(STAT_GAME_COMPLETION_AS_BALL) >= 1 &&
                get_statistic(STAT_GAME_COMPLETION_AS_CAMO) >= 1 &&
                get_statistic(STAT_GAME_COMPLETION_AS_BRAWLER) >= 1 &&
                get_statistic(STAT_GAME_COMPLETION_AS_SHIELD) >= 1 &&
                get_statistic(STAT_ENEMIES_KILLED) >= 500) {
                set_achievement_progress(ACH_HERO_OF_NADAROTH, 1);
            }
            pause_game();
            lock_inputs();
        }
        play_cinematic(data->screen, filepath, CINEMATIC_FRAME_DELAY);
        if (phase == GAMEPHASE_FIRST_ACT_END) play_cinematic(data->screen, "assets/cinematics/wip.dodjo", CINEMATIC_FRAME_DELAY);  //! Placeholder for future content
        if (get_player_mental_health(data->p) == 0) {
            play_cinematic(data->screen, "assets/cinematics/the_end.dodjo", CINEMATIC_FRAME_DELAY);
            pause_game();
            lock_inputs();
        }
    }

    free(data);
}

bool spawn_projectile(int x0, int y0, int x1, int y1, int from, int rate, unsigned int design, int range, bool infinity, ProjectileCallback callback, projectile_data* callback_data) {
    bool spawned = false;
    pthread_mutex_lock(&projectile_mutex);
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!projectiles[i].active) {
            projectiles[i] = (Projectile){
                .x = x0 / XSPACING,
                .y = y0,
                .x1 = x1 / XSPACING,
                .y1 = y1,
                .dx = abs(x1 - x0) / XSPACING,
                .dy = -abs(y1 - y0),
                .sx = x0 < x1 ? 1 : -1,
                .sy = y0 < y1 ? 1 : -1,
                .err = (abs(x1 - x0) / XSPACING) + (-abs(y1 - y0)),
                .from = from,
                .frame = 0,
                .rate = rate,
                .design = design,
                .active = true,
                .home = true,
                .range = range,
                .distance_traveled = 0,
                .infinity = infinity,
                .callback = callback,
                .callback_data = callback_data};
            draw_ppos_if_nothing_here(callback_data->screen, &projectiles[i], projectiles[i].design);
            spawned = true;
            break;
        }
    }
    pthread_mutex_unlock(&projectile_mutex);
    return spawned;
}

void fire_projectile(Render_Buffer* r, player* p, int target_x, int target_y) {
    if (get_difficulty() == DIFFICULTY_HARD && total_player_projectiles >= 1) return;
    player_update_weapon(p);
    int x = get_player_x(p) + RECENTER_X - PADDING_X;
    int y = -get_player_y(p) + RECENTER_Y;

    if (x == target_x && y == target_y) return;

    projectile_data* p_data = malloc(sizeof(projectile_data));
    p_data->x0 = x;
    p_data->y0 = y;
    p_data->damage = get_player_damage(p);
    p_data->p = p;
    p_data->screen = r;

    total_player_projectiles++;

    spawn_projectile(
        x,
        y,
        target_x,
        target_y,
        get_player_design(p),
        get_player_arrow_speed(p),
        PLAYER_PROJECTILE_DESIGN,
        get_player_range(p),
        has_infinity(p),
        projectile_callback,
        p_data);
}

void* projectile_loop(void* args) {
    InitThreadArgs* arg = (InitThreadArgs*)args;
    Render_Buffer* r = arg->r;
    player* p = arg->p;

    int* enemy_attack_timers = NULL;
    chunk* current_chunk = NULL;
    int* enemy_ids = NULL;

    struct timespec ts = {.tv_sec = 0, .tv_nsec = 16666667};  // 60 FPS

    while (1) {
        if (projectile_thread_stop) {
            break;
        }
        pthread_mutex_lock(&pause_mutex);
        while (GAME_PAUSED && !projectile_thread_stop) {
            pthread_cond_wait(&pause_cond, &pause_mutex);
        }
        pthread_mutex_unlock(&pause_mutex);

        if (projectile_thread_stop) {
            break;
        }

        chunk* c = get_player_chunk(p);
        dynarray* d = get_chunk_enemies(c);
        int current_enemy_count = len_dyn(d);

        if (c != current_chunk || current_enemy_count != tracked_enemy_count) {
            free(enemy_attack_timers);
            free(enemy_ids);
            current_chunk = c;
            tracked_enemy_count = current_enemy_count;
            enemy_attack_timers = malloc(current_enemy_count * sizeof(int));
            enemy_ids = malloc(current_enemy_count * sizeof(int));
            total_player_projectiles = 0;

            for (int i = 0; i < current_enemy_count; i++) {
                enemy_attack_timers[i] = rand_r(&projectile_rng_seed) % FIRING_RANDOM_OFFSET_MAX + (get_difficulty() == DIFFICULTY_EASY ? EASY_MODE_INV_FRAMES : HARD_MODE_INV_FRAMES);  // 60-90 frames before starting to attack
                enemy_ids[i] = i;
            }
        }

        for (int i = 0; i < current_enemy_count; i++) {
            if (enemy_ids == NULL || enemy_attack_timers == NULL) continue;

            item* brain = get_dyn(d, enemy_ids[i]);
            if (brain != NULL) {
                enemy_attack_timers[i]--;

                if (enemy_attack_timers[i] <= 0) {
                    enemy* enemy_brain = (enemy*)get_item_spec(brain);
                    attack_fn attack = get_attack_fn(enemy_brain->entity_type);
                    attack(r, p, brain, &enemy_attack_timers[i]);
                    if (enemy_attack_timers[i] == -1) enemy_attack_timers[i] = rand_r(&projectile_rng_seed) % enemy_brain->attack_interval + enemy_brain->attack_delay;  // delay ~> delay+interval frames
                }
            }
        }

        update_projectiles(r);
        nanosleep(&ts, NULL);
    }

    free(enemy_attack_timers);
    free(enemy_ids);
    free(arg);
    return NULL;
}

static void start_projectile_thread(Render_Buffer* r, player* p, int seed, bool reset_state) {
    if (!projectile_mutex_ready) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&projectile_mutex, &attr);
        pthread_mutexattr_destroy(&attr);
        projectile_mutex_ready = true;
    }

    InitThreadArgs* input_args = malloc(sizeof(InitThreadArgs));
    input_args->r = r;
    input_args->p = p;
    projectile_rng_seed = seed * 31 + 1;

    if (reset_state) {
        struct timeval started;
        gettimeofday(&started, NULL);
        set_game_started(started);
        add_total_enemies(p);
    }

    if (pthread_create(&projectile_thread, NULL, projectile_loop, input_args) != 0) {
        LOG_ERROR("Failed to create projectile thread\n");
        exit(EXIT_FAILURE);
    }
    projectile_thread_running = true;
}

void init_projectile_system(Render_Buffer* r, player* p, int seed) {
    projectile_thread_stop = false;
    start_projectile_thread(r, p, seed, true);
}

void stop_projectile_system() {
    if (!projectile_thread_running) {
        LOG_INFO("Can't stop projectile system : already stopped");
        return;
    }

    projectile_thread_stop = true;
    pthread_cond_broadcast(&pause_cond);
    pthread_join(projectile_thread, NULL);
    projectile_thread_running = false;
    projectile_thread_stop = false;

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].active) {
            if (projectiles[i].callback_data) {
                free(projectiles[i].callback_data);
                projectiles[i].callback_data = NULL;
            }
            projectiles[i].active = false;
        }
    }

    LOG_INFO("Stopped projectile system");
}

void restart_projectile_system(Render_Buffer* r, player* p, int seed) {
    if (projectile_thread_running) {
        stop_projectile_system();
    }

    projectile_thread_stop = false;
    start_projectile_thread(r, p, seed, false);
}

void simulate_projectile_hit(int damage, player* p, Render_Buffer* screen) {
    projectile_data* p_data = malloc(sizeof(projectile_data));
    p_data->damage = damage;
    p_data->p = p;
    p_data->screen = screen;

    enemy_attack_callback(get_player_x(p), get_player_y(p), p_data);
}

void add_total_enemies(player* p) {
    chunk* c = get_player_chunk(p);
    dynarray* d = get_chunk_enemies(c);
    int count = len_dyn(d);
    total_enemies += count;
}

void reset_total_enemies() {
    total_enemies = 0;
    tracked_enemy_count = -1;
}