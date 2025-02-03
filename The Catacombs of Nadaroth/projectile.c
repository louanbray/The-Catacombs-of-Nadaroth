#include "projectile.h"

#include <pthread.h>

#include "constants.h"
#include "entity.h"
#include "player.h"
#include "render.h"

#define MAX_PROJECTILES 128

typedef struct projectile_data {
    int x0, y0;
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
    bool active;                     // Is the projectile active?
    ProjectileCallback callback;     // Callback to execute on hit
    projectile_data* callback_data;  // Additional data for callback
} Projectile;

Projectile projectiles[MAX_PROJECTILES];
pthread_mutex_t projectile_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t entity_mutex = PTHREAD_MUTEX_INITIALIZER;

// Bresenham's Line Algorithm
void update_projectiles(Render_Buffer* r) {
    pthread_mutex_lock(&projectile_mutex);

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!projectiles[i].active) continue;

        Projectile* p = &projectiles[i];

        wchar_t c = render_get_cell_char(r, RENDER_HEIGHT - p->y, p->x * 2 - 1);

        if (c == L' ') {
            wprintf(L"\033[%d;%dH ", p->y, p->x * 2);
        }

        if (c != 3486 && ((p->x == p->x1 && p->y == p->y1) || c != L' ')) {
            if (p->callback) {
                p->callback(p->x * 2 - 65, 19 - p->y, p->callback_data);
            }

            p->active = false;
            continue;
        }

        int e2 = 2 * p->err;
        if (e2 >= p->dy) {
            p->err += p->dy;
            p->x += p->sx;
        }
        if (e2 <= p->dx) {
            p->err += p->dx;
            p->y += p->sy;
        }

        c = render_get_cell_char(r, RENDER_HEIGHT - p->y, p->x * 2 - 1);

        if (c == L' ') {
            wprintf(L"\033[%d;%dH*", p->y, p->x * 2);
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

            e->hp -= 1;

            if (e->hp <= 0) {
                if (is_entity) {
                    destroy_entity_from_chunk(ent);
                    render_from_player(data->screen, data->p);
                } else {
                    remove_item(get_player_chunk(data->p), it);
                    render_char(get_board(data->screen), x, y, ' ');
                }
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

void* projectile_loop(void* args) {
    Render_Buffer* r = (Render_Buffer*)args;

    struct timespec ts = {.tv_sec = 0, .tv_nsec = 16666667};  // ~60 FPS
    while (1) {
        update_projectiles(r);
        nanosleep(&ts, NULL);
    }

    return NULL;
}

void spawn_projectile(int x0, int y0, int x1, int y1, ProjectileCallback callback, projectile_data* callback_data) {
    pthread_mutex_lock(&projectile_mutex);
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!projectiles[i].active) {
            projectiles[i] = (Projectile){
                .x = x0 / 2,
                .y = y0,
                .x1 = x1 / 2,
                .y1 = y1,
                .dx = abs(x1 - x0) / 2,
                .dy = -abs(y1 - y0),
                .sx = x0 < x1 ? 1 : -1,
                .sy = y0 < y1 ? 1 : -1,
                .err = (abs(x1 - x0) / 2) + (-abs(y1 - y0)),
                .active = true,
                .callback = callback,
                .callback_data = callback_data};
            break;
        }
    }
    pthread_mutex_unlock(&projectile_mutex);
}

void fire_projectile(Render_Buffer* r, player* p, int target_x, int target_y) {
    int x = get_player_x(p) + 65;
    int y = -get_player_y(p) + 19;

    if (x == target_x && y == target_y) return;

    projectile_data* p_data = malloc(sizeof(projectile_data));
    p_data->x0 = x;
    p_data->y0 = y;
    p_data->p = p;
    p_data->screen = r;

    spawn_projectile(
        x,
        y,
        target_x,
        target_y,
        projectile_callback,
        p_data);
}

void init_projectile_system(Render_Buffer* r) {
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, projectile_loop, r) != 0) {
        fprintf(stderr, "Failed to create projectile thread\n");
        exit(EXIT_FAILURE);
    }
    pthread_detach(thread_id);
}