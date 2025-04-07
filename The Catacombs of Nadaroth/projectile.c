#include "projectile.h"

#include "assets_manager.h"
#include "constants.h"
#include "entity.h"
#include "game_status.h"
#include "player.h"
#include "render.h"

#define MAX_PROJECTILES 128

int last_hotbar_index = 0;

typedef struct {
    Render_Buffer* r;
    player* p;
} InitThreadArgs;

typedef struct projectile_data {
    int x0, y0, damage;
    player* p;
    Render_Buffer* screen;
} projectile_data;

typedef void (*ProjectileCallback)(int x, int y, projectile_data* data);

typedef struct Projectile {
    int x, y;                        // Current position
    int x1, y1;                      // Target position
    int dx, dy;                      // Absolute differences
    int sx, sy;                      // Step directions
    int err;                         // Error term
    int from, from_id;               // Character to ignore
    int frame, rate;                 // Animation frame and rate
    bool infinity;                   // Is the projectile infinite?
    bool home;                       // Is the projectile homing?
    bool active;                     // Is the projectile active?
    ProjectileCallback callback;     // Callback to execute on hit
    projectile_data* callback_data;  // Additional data for callback
} Projectile;

Projectile projectiles[MAX_PROJECTILES];
pthread_mutex_t projectile_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t entity_mutex = PTHREAD_MUTEX_INITIALIZER;

void kill_all_projectiles(Render_Buffer* r) {
    pthread_mutex_lock(&projectile_mutex);
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        Projectile* p = &projectiles[i];
        if (p->active) {
            wchar_t c = render_get_cell_char(r, RENDER_HEIGHT - p->y, p->x * 2 - 1);

            if (c == L' ') {
                wprintf(L"\033[%d;%dH ", p->y, p->x * 2);
            }
        }
        p->active = false;
    }
    pthread_mutex_unlock(&projectile_mutex);
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

        int speed = 2;

        wchar_t c = render_get_cell_char(r, RENDER_HEIGHT - p->y, p->x * speed - 1);

        if (c == L' ') {
            wprintf(L"\033[%d;%dH ", p->y, p->x * speed);
        }

        if ((c != p->from || !p->home) && (((p->x == p->x1 && p->y == p->y1) && !p->infinity) || c != L' ')) {
            if (p->callback) {
                p->callback(p->x * speed - 65, 19 - p->y, p->callback_data);
            }

            p->active = false;
            continue;
        }

        int e2 = speed * p->err;
        if (e2 >= p->dy) {
            p->err += p->dy;
            p->x += p->sx;
        }
        if (e2 <= p->dx) {
            p->err += p->dx;
            p->y += p->sy;
        }

        c = render_get_cell_char(r, RENDER_HEIGHT - p->y, p->x * speed - 1);

        if (c == L' ') {
            if (p->home)
                p->home = false;
            wprintf(L"\033[%d;%dH*", p->y, p->x * speed);
        }
    }

    pthread_mutex_unlock(&projectile_mutex);
    fflush(stdout);
}

// Function to animate a projectile moving from (x0, y0) to (x1, y1)
void projectile_callback(int x, int y, projectile_data* data) {
    item* it = get_hm(get_chunk_furniture_coords(get_player_chunk(data->p)), x, y);

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
        case ENEMY: {
            enemy* e = get_item_spec(it);

            pthread_mutex_lock(&entity_mutex);

            e->hp -= data->damage;

            if (e->hp <= 0) {
                set_dyn(get_chunk_enemies(get_player_chunk(data->p)), e->from_id, NULL);
                if (is_entity) {
                    destroy_entity_from_chunk(ent);
                    render_from_player(data->screen, data->p);
                } /*else {
                    remove_item(get_player_chunk(data->p), it);
                    render_char(get_board(data->screen), x, y, ' ');
                }*/
            }

            pthread_mutex_unlock(&entity_mutex);

            break;
        }

        default:
            break;
    }

    update_screen(data->screen);
    free(data);
}

void enemy_attack_callback(int x, int y, projectile_data* data) {
    if (x == get_player_x(data->p) && y == get_player_y(data->p)) {
        damage_player(data->p, data->damage);
        render_health(data->screen, data->p);
    }

    update_screen(data->screen);
    free(data);
}

void spawn_projectile(int x0, int y0, int x1, int y1, int from, int rate, bool infinity, ProjectileCallback callback, projectile_data* callback_data) {
    pthread_mutex_lock(&projectile_mutex);
    int speed = 2;
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!projectiles[i].active) {
            projectiles[i] = (Projectile){
                .x = x0 / speed,
                .y = y0,
                .x1 = x1 / speed,
                .y1 = y1,
                .dx = abs(x1 - x0) / speed,
                .dy = -abs(y1 - y0),
                .sx = x0 < x1 ? 1 : -1,
                .sy = y0 < y1 ? 1 : -1,
                .err = (abs(x1 - x0) / speed) + (-abs(y1 - y0)),
                .from = from,
                .frame = 0,
                .rate = rate,
                .active = true,
                .home = true,
                .infinity = infinity,
                .callback = callback,
                .callback_data = callback_data};
            break;
        }
    }
    pthread_mutex_unlock(&projectile_mutex);
}

void enemy_attack_projectile(Render_Buffer* r, player* p, item* brain) {
    int x = get_item_x(brain) + 65;
    int y = -get_item_y(brain) + 19;

    projectile_data* p_data = malloc(sizeof(projectile_data));
    p_data->x0 = x;
    p_data->y0 = y;
    p_data->damage = ((enemy*)get_item_spec(brain))->damage;
    p_data->p = p;
    p_data->screen = r;

    spawn_projectile(
        x,
        y,
        get_player_x(p) + 65,
        -get_player_y(p) + 19,
        get_item_display(brain),
        ((enemy*)get_item_spec(brain))->speed,
        ((enemy*)get_item_spec(brain))->infinity,
        enemy_attack_callback,
        p_data);
}

void bow_check_flag() {
    last_hotbar_index = -1;
}

void fire_projectile(Render_Buffer* r, player* p, int target_x, int target_y) {
    int index = get_selected_slot(get_player_hotbar(p));
    if (index != last_hotbar_index) {
        last_hotbar_index = index;
        item* it = get_selected_item(get_player_hotbar(p));
        set_player_damage(p, 1);
        if (it != NULL) {
            UsableItem type = get_item_usable_type(it);
            if (type == BASIC_BOW || type == ADVANCED_BOW || type == SUPER_BOW || type == NADINO_BOW) {
                set_player_damage(p, get_usable_item_file(type)->specs.specs[1]);
            }
        }
    }
    int x = get_player_x(p) + 65;
    int y = -get_player_y(p) + 19;

    if (x == target_x && y == target_y) return;

    projectile_data* p_data = malloc(sizeof(projectile_data));
    p_data->x0 = x;
    p_data->y0 = y;
    p_data->damage = get_player_damage(p);
    p_data->p = p;
    p_data->screen = r;

    spawn_projectile(
        x,
        y,
        target_x,
        target_y,
        get_player_design(p),
        1,
        true,
        projectile_callback,
        p_data);
}

void* projectile_loop(void* args) {
    InitThreadArgs* arg = (InitThreadArgs*)args;
    Render_Buffer* r = arg->r;
    player* p = arg->p;

    struct timespec ts = {.tv_sec = 0, .tv_nsec = 16666667};  // 60 FPS
    int tick = 0;
    while (1) {
        pthread_mutex_lock(&pause_mutex);
        while (GAME_PAUSED) {
            pthread_cond_wait(&pause_cond, &pause_mutex);
        }
        pthread_mutex_unlock(&pause_mutex);

        // TODO : Add enemies to a pool so that the update can be separated for each enemy (+ implement spec (2 more needed) for time between attacks (ex : 180 +- 60))
        if (tick == 60) {
            tick = 0;
            chunk* c = get_player_chunk(p);
            dynarray* d = get_chunk_enemies(c);
            for (int i = 0; i < len_dyn(d); i++) {
                item* brain = get_dyn(d, i);
                if (brain != NULL)
                    enemy_attack_projectile(r, p, brain);
            }
        }
        update_projectiles(r);

        nanosleep(&ts, NULL);
        tick++;
    }

    if (arg)
        free(arg);

    return NULL;
}

void init_projectile_system(Render_Buffer* r, player* p) {
    pthread_t thread_id;
    InitThreadArgs* input_args = malloc(sizeof(InitThreadArgs));
    input_args->r = r;
    input_args->p = p;
    if (pthread_create(&thread_id, NULL, projectile_loop, input_args) != 0) {
        fprintf(stderr, "Failed to create projectile thread\n");
        exit(EXIT_FAILURE);
    }
    pthread_detach(thread_id);
}