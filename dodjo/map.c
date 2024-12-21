#include "map.h"

#include <stdio.h>
#include <time.h>

#include "dynarray.h"

/// @brief chunk type (0,0) -> SPAWN
enum Type {
    SPAWN
};

typedef struct chunk {
    chunk_link link;
    int x;
    int y;
    int type;
    dynarray* elements;
    hm* hashmap;
} chunk;

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
    ck->type = SPAWN;  // TODO: ADD RANDOM TYPEGEN
    ck->elements = NULL;
    ck->hashmap = create_hashmap();
    return ck;
}

chunk* generate_chunk(int x, int y) {
    return create_chunk(x, y);
}

map* create_map(player* p) {
    map* m = malloc(sizeof(map));
    chunk* ck = create_chunk(0, 0);

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

dynarray* get_chunk_furniture_list(chunk* ck) {
    return ck->elements;
}

hm* get_chunk_furniture_coords(chunk* ck) {
    return ck->hashmap;
}

int get_chunk_x(chunk* c) {
    return c->x;
}

int get_chunk_y(chunk* c) {
    return c->y;
}

void set_chunk_type(chunk* ck, int type) {
    ck->type = type;
}

void set_map_player(map* m, player* p) {
    m->player = p;
}

chunk* get_chunk(map* m, int x, int y) {
    chunk* ck = get_hm(m->hashmap, x, y);
    if (ck != NULL) {
        return ck;
    }
    ck = create_chunk(x, y);
    set_hm(m->hashmap, x, y, ck);
    return ck;
}

chunk* get_chunk_from(map* m, chunk* c1, int dir) {
    if (c1->link[dir] != NULL) {
        return c1->link[dir];
    }
    if (dir != 0) {
        int s = dir < 3 ? 1 : -1;
        chunk* ck = get_chunk(m, c1->x + dir % 2 * s, c1->y + s * (dir - 1) % 2);
        ck->link[(dir + 2 * s)] = c1;
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
    purge_hm(m, ck->x, ck->y);
    for (int i = 0; i < 5; i++) {
        int s = i == 0 ? 0 : (i < 3 ? 1 : -1);
        if (ck->link[i] != NULL) {
            ck->link[i]->link[(i + 2 * s)] = NULL;
        }
    }
    free(ck->link);
    free_dyn(ck->elements);
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

//
// int main() {
//     srand(time(NULL));
//     map* m = create_map();
//     print_map(m);
//     chunk* ck = get_chunk_from(m, m->spawn, STARGATE);
//     /*print_map(m);
//     print_chunk(ck);*/
//     printf("(x: %d, y: %d) -> ", ck->x, ck->y);
//     for (int i = 0; i < 50; i++) {
//         int d = rand() % 5;
//         ck = get_chunk_from(m, ck, d);
//         if (!d) {
//             printf("(x: %d, y: %d) -> ", ck->x, ck->y);
//         } else {
//             printf("[...]");
//         }
//         /*print_map(m);
//         print_chunk(ck);*/
//     }
//     printf("\n");
//     print_map(m);
//     print_chunk(ck);
// }