#include "map.h"

#include <stdio.h>
#include <stdlib.h>

#include "achievements.h"
#include "statistics.h"

typedef struct map {
    hm* hashmap;
    chunk* spawn;
    player* player;
} map;

static bool IS_NEW_CHUNK = false;

/// @brief Create a core map bound the the player
/// @param p player
/// @return map
map* create_map() {
    map* m = malloc(sizeof(map));
    chunk* ck = generate_chunk(0, 0);

    m->hashmap = create_hashmap();
    m->spawn = ck;
    m->player = NULL;
    set_hm(m->hashmap, 0, 0, ck);
    return m;
}

chunk* get_spawn(map* m) {
    return m->spawn;
}

player* get_player(map* m) {
    return m->player;
}

void set_map_player(map* m, player* p) {
    m->player = p;
}

hm* get_map_hashmap(map* m) {
    return m->hashmap;
}

bool is_new_chunk() {
    return IS_NEW_CHUNK;
}

void reset_new_chunk_flag() {
    IS_NEW_CHUNK = false;
}

chunk* get_chunk(map* m, int x, int y) {
    chunk* ck = get_hm(m->hashmap, x, y);

    if (ck != NULL) {
        IS_NEW_CHUNK = false;
        return ck;
    }

    IS_NEW_CHUNK = true;
    if (get_achievement_progress(ACH_MASTER_EXPLORER) < 10)
        add_achievement_progress(ACH_MASTER_EXPLORER, 1);
    if (get_achievement_progress(ACH_MASTER_EXPLORER) % 10 == 0 && get_statistic(STAT_DISTANCE_TRAVELED) >= 10000) {
        add_achievement_progress(ACH_MASTER_EXPLORER, 1);
    }
    ck = generate_chunk(x, y);
    set_hm(m->hashmap, x, y, ck);

    return ck;
}

chunk* get_chunk_from(map* m, chunk* c1, Direction dir) {
    if (get_chunk_link_at(c1, dir) != NULL) {
        return get_chunk_link_at(c1, dir);
    }

    if (dir != 0) {
        const int dx[] = {0, 1, 0, -1, 0};
        const int dy[] = {0, 0, 1, 0, -1};
        int s = dir < 3 ? 2 : -2;

        chunk* ck = get_chunk(m, get_chunk_x(c1) + dx[dir], get_chunk_y(c1) + dy[dir]);

        set_chunk_link(ck, dir + s, c1);
        set_chunk_link(c1, dir, ck);

        return ck;
    } else {
        int x = rand() % 20 - 10;
        int y = rand() % 20 - 10;
        if (x == get_chunk_x(c1) && y == get_chunk_y(c1)) {
            x += 1;
            y += 1;
        }

        chunk* ck = get_chunk(m, get_chunk_x(c1) + x, get_chunk_y(c1) + y);

        set_chunk_link(ck, dir, c1);
        set_chunk_link(c1, dir, ck);

        return ck;
    }
}

/// @brief Free full chunk in the map and itself (FUNCTION PASSED TO HASHMAP)
/// @param m map
/// @param ck chunk to purge
void purge_chunk(hm* m, chunk* ck) {
    //? Allow delete spawn ?
    if (get_chunk_x(ck) == 0 && get_chunk_y(ck) == 0) return;

    purge_hm(m, get_chunk_x(ck), get_chunk_y(ck));
    for (int i = 0; i < 5; i++) {
        int s = i == 0 ? 0 : (i < 3 ? 2 : -2);
        chunk* linked = get_chunk_link_at(ck, i);
        if (linked != NULL) {
            set_chunk_link(linked, i + s, NULL);
        }
    }

    destroy_chunk_full(ck);
}

void destroy_chunk(map* m, chunk* ck) {
    purge_chunk(m->hashmap, ck);
}

void print_chunk(chunk* ck) {
    chunk_link lk = get_chunk_links(ck);
    printf("CHUNK: %p [x: %d, y: %d, type: %d, element: %p]\n", (void*)ck,
           get_chunk_x(ck), get_chunk_y(ck), get_chunk_type(ck), (void*)get_chunk_furniture_list(ck));
    printf("link: [%p, %p, %p, %p, %p]\n\n",
           (void*)lk[0], (void*)lk[1], (void*)lk[2], (void*)lk[3], (void*)lk[4]);
}

void print_map(map* m) {
    print_hm(m->hashmap);
    print_chunk(m->spawn);
}

/// @brief Callback used by destroy_map to free each chunk's internals
static void free_chunk_callback(int key_x, int key_y, element_h elem, void* user_data) {
    (void)key_x;
    (void)key_y;
    (void)user_data;
    destroy_chunk_full((chunk*)elem);
}

void destroy_map(map* m) {
    if (m == NULL) return;
    for_each_hm(m->hashmap, free_chunk_callback, NULL);
    free_hm(m->hashmap);
    free(m);
}