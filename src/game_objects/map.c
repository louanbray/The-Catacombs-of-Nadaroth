#include "map.h"

#include <stdio.h>
#include <stdlib.h>

#include "../managers/achievements_manager.h"
#include "../managers/save_manager.h"
#include "../managers/statistics_manager.h"
#include "../utils/logger.h"

typedef struct map {
    hm* hashmap;
    hm* cache_map;
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
    m->cache_map = create_hashmap();
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

hm* get_map_cache_hashmap(map* m) {
    return m ? m->cache_map : NULL;
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

    if (is_chunk_in_cache(m, x, y)) {
        IS_NEW_CHUNK = false;
        ck = load_chunk_from_cache(m, x, y);
        if (ck != NULL) {
            set_hm(m->hashmap, x, y, ck);
            return ck;
        }
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

typedef struct unload_callback_data {
    map* m;
    int player_x;
    int player_y;
    int count;
    int keys_x[256];
    int keys_y[256];
} unload_callback_data;

static void collect_far_chunks_cb(int key_x, int key_y, element_h elem, void* user_data) {
    (void)elem;
    unload_callback_data* data = (unload_callback_data*)user_data;
    if (!data) return;

    if (key_x == 0 && key_y == 0) return;

    int dx = abs(key_x - data->player_x);
    int dy = abs(key_y - data->player_y);
    int dist = (dx > dy) ? dx : dy;

    if (dist > 3 && data->count < 256) {
        data->keys_x[data->count] = key_x;
        data->keys_y[data->count] = key_y;
        data->count++;
    }
}

void update_chunk_unloading(map* m, int player_chunk_x, int player_chunk_y) {
    if (!m || !m->hashmap) return;
    unload_callback_data data = {0};
    data.m = m;
    data.player_x = player_chunk_x;
    data.player_y = player_chunk_y;

    for_each_hm(m->hashmap, collect_far_chunks_cb, &data);

    for (int i = 0; i < data.count; i++) {
        int x = data.keys_x[i];
        int y = data.keys_y[i];
        chunk* ck = get_hm(m->hashmap, x, y);
        if (ck) {
            for (int d = 0; d < 5; d++) {
                chunk* neighbor = get_chunk_link_at(ck, d);
                if (neighbor) {
                    for (int r = 0; r < 5; r++) {
                        if (get_chunk_link_at(neighbor, r) == ck) {
                            set_chunk_link(neighbor, r, NULL);
                        }
                    }
                }
            }
            save_chunk_to_cache(m, ck);
            purge_hm(m->hashmap, x, y);
            destroy_chunk_full(ck);
        }
    }
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
    LOG_S("CHUNK: %p [x: %d, y: %d, type: %d, element: %p]\n", (void*)ck,
          get_chunk_x(ck), get_chunk_y(ck), get_chunk_type(ck), (void*)get_chunk_furniture_list(ck));
    LOG_S("link: [%p, %p, %p, %p, %p]\n\n",
          (void*)lk[0], (void*)lk[1], (void*)lk[2], (void*)lk[3], (void*)lk[4]);
}

void print_map(map* m) {
    print_hm(m->hashmap);
    print_chunk(m->spawn);
}

static void free_chunk_callback(int key_x, int key_y, element_h elem, void* user_data) {
    (void)key_x;
    (void)key_y;
    (void)user_data;
    destroy_chunk_full((chunk*)elem);
}

static void free_cache_entry_callback(int key_x, int key_y, element_h elem, void* user_data) {
    (void)key_x;
    (void)key_y;
    (void)user_data;
    if (elem) free(elem);
}

void destroy_map(map* m) {
    if (m == NULL) return;
    for_each_hm(m->hashmap, free_chunk_callback, NULL);
    free_hm(m->hashmap);
    if (m->cache_map) {
        for_each_hm(m->cache_map, free_cache_entry_callback, NULL);
        free_hm(m->cache_map);
    }
    free(m);
}