#include "map.h"

#include <stdio.h>

#include "generation.h"

typedef struct map {
    hm* hashmap;
    chunk* spawn;
    player* player;
} map;

/// @brief Create an empty set of links
/// @return the link set
chunk_link create_link() {
    chunk_link lk = calloc(5, sizeof(chunk*));
    return lk;
}

/// @brief Create empty chunk (with a random type)
/// @param x chunk coord x
/// @param y chunk coord y
/// @return created chunk
chunk* create_chunk(int x, int y) {
    chunk* ck = malloc(sizeof(chunk));
    ck->link = create_link();
    ck->x = x;
    ck->y = y;
    ck->type = SPAWN;
    ck->elements = create_dyn();
    ck->enemies = create_dyn();
    ck->hashmap = create_hashmap();
    return ck;
}

/// @brief Generate a random decorated chunk
/// @param x chunk x
/// @param y chunk y
/// @return generated chunk
chunk* generate_chunk(int x, int y) {
    chunk* c = create_chunk(x, y);
    decorate(c, x, y);
    return c;
}

/// @brief Create a core map bound the the player
/// @param p player
/// @return map
map* create_map(player* p) {
    map* m = malloc(sizeof(map));
    chunk* ck = generate_chunk(0, 0);

    m->hashmap = create_hashmap();
    m->spawn = ck;
    m->player = p;
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

chunk* get_chunk(map* m, int x, int y) {
    chunk* ck = get_hm(m->hashmap, x, y);

    if (ck != NULL) {
        return ck;
    }

    ck = generate_chunk(x, y);
    set_hm(m->hashmap, x, y, ck);

    return ck;
}

chunk* get_chunk_from(map* m, chunk* c1, Direction dir) {
    if (c1->link[dir] != NULL) {
        return c1->link[dir];
    }

    if (dir != 0) {
        const int dx[] = {0, 1, 0, -1, 0};
        const int dy[] = {0, 0, 1, 0, -1};
        int s = dir < 3 ? 2 : -2;

        chunk* ck = get_chunk(m, c1->x + dx[dir], c1->y + dy[dir]);

        ck->link[dir + s] = c1;
        c1->link[dir] = ck;

        return ck;
    } else {
        int x = rand() % 20 - 10;
        int y = rand() % 20 - 10;
        if (x == c1->x && y == c1->y) {
            x += 1;
            y += 1;
        }

        chunk* ck = get_chunk(m, c1->x + x, c1->y + y);

        ck->link[dir] = c1;
        c1->link[dir] = ck;

        return ck;
    }
}

/// @brief Free full chunk in the map and itself (FUNCTION PASSED TO HASHMAP)
/// @param m map
/// @param ck chunk to purge
void purge_chunk(hm* m, chunk* ck) {
    //? Allow delete spawn ?
    if (ck->x == 0 && ck->y == 0) return;

    purge_hm(m, ck->x, ck->y);
    for (int i = 0; i < 5; i++) {
        int s = i == 0 ? 0 : (i < 3 ? 2 : -2);

        if (ck->link[i] != NULL) {
            ck->link[i]->link[(i + s)] = NULL;
        }
    }

    free(ck->link);
    free_dyn(ck->elements);
    free_dyn(ck->enemies);
    free_hm(ck->hashmap);
    free(ck);
}

void destroy_chunk(map* m, chunk* ck) {
    purge_chunk(m->hashmap, ck);
}

void print_chunk(chunk* ck) {
    printf("CHUNK: %p [", ck);
    printf("x: %d, y: %d, type: %d, element: %p]\n", ck->x, ck->y, ck->type, ck->elements);
    if (false) {
        printf("link (%p): [%p, %p, %p, %p, %p]\n\n", ck->link, ck->link[0], ck->link[1], ck->link[2], ck->link[3], ck->link[4]);
    } else {
        printf("link: [%p, %p, %p, %p, %p]\n\n", ck->link[0], ck->link[1], ck->link[2], ck->link[3], ck->link[4]);
    }
}

void print_map(map* m) {
    print_hm(m->hashmap);
    print_chunk(m->spawn);
}