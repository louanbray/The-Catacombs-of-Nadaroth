#include <pthread.h>
#include <time.h>

#include "entity.h"
#include "input_manager.h"
#include "map.h"
#include "player.h"
#include "render.h"

#define CHAR_TO_INT 49

pthread_mutex_t entity_mutex = PTHREAD_MUTEX_INITIALIZER;

// Struct to hold arguments for the thread
typedef struct {
    int x0, y0, x1, y1;
    player* p;
    Render_Buffer* r;
} RenderArgs;

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

// Function to animate a projectile moving from (x0, y0) to (x1, y1)
void animate_projectile(int x0, int y0, int x1, int y1, player* p, Render_Buffer* screen) {
    int x = 0, y = -101;

    render_projectile(x0, y0, x1, y1, &x, &y, screen);
    fflush(stdout);

    if (y == -101) return;

    item* it = get_hm(get_chunk_furniture_coords(get_player_chunk(p)), x, y);

    if (it == NULL) return;

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
                    render_from_player(screen, p);
                } else {
                    remove_item(get_player_chunk(p), it);
                    render_char(get_board(screen), x, y, ' ');
                }
            }

            pthread_mutex_unlock(&entity_mutex);

            break;
        }

        default:
            break;
    }

    update_screen(screen);
}

void* animate_projectile_thread(void* args) {
    RenderArgs* renderArgs = (RenderArgs*)args;
    animate_projectile(renderArgs->x0, renderArgs->y0, renderArgs->x1, renderArgs->y1,
                       renderArgs->p, renderArgs->r);
    free(renderArgs);
    return NULL;
}

void on_left_click(Render_Buffer* screen, player* p, int x, int y) {
    RenderArgs* args = malloc(sizeof(RenderArgs));
    if (!args) exit(EXIT_FAILURE);  // Failed to allocate memory

    *args = (RenderArgs){get_player_x(p) + 65, -get_player_y(p) + 19, x, y, p, screen};

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, animate_projectile_thread, args) != 0) {  // Failed to create thread
        free(args);
        exit(EXIT_FAILURE);
    }

    pthread_detach(thread_id);
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

    process_input(p, screen, on_left_click, arrow_move, compute_entry);

    return EXIT_SUCCESS;
}