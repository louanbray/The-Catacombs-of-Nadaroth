#include "map.h"

#include <stdio.h>
#include <time.h>

#include "hash.h"

/// @brief chunk type (0,0) -> SPAWN
enum Type {
    SPAWN
};

/// @brief Gate position/type
enum Direction {
    STARGATE,
    EAST,
    NORTH,
    WEST,
    SOUTH
};
/// @brief Player
typedef struct player player;
/// @brief define link to an array of chunk*
typedef chunk** link;
typedef struct chunk {
    link link;
    int x;
    int y;
    int type;
    dynarray* elements;
} chunk;

typedef struct map {
    hm* hashmap;
    chunk* spawn;
    player* player;
} map;

/// @brief Create an empty set of links
/// @return the link set
link create_link() {
    link lk = calloc(5, sizeof(chunk*));
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
    return ck;
}

/// @brief Create a map, a hashmap of chunk* with spawn value
/// @param free_fun Free elements function
/// @return map
map* create_map(player* p) {
    map* m = malloc(sizeof(map));
    hm* hashmap = create_hashmap();

    m->hashmap = hashmap;
    chunk* ck = create_chunk(0, 0);
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

dynarray* get_chunk_furniture(chunk* ck) {
    return ck->elements;
}

void set_chunk_type(chunk* ck, int type) {
    ck->type = type;
}

/// @brief Get the chunk in x,y or create it
/// @param m map
/// @param x chunk x
/// @param y chunk y
/// @return accessed or created chunk
chunk* getChunk(map* m, int x, int y) {
    chunk* ck = get_hm(m->hashmap, x, y);
    if (ck != NULL) {
        return ck;
    }
    ck = create_chunk(x, y);
    set_hm(m->hashmap, x, y, ck);
    return ck;
}

/// @brief Get the loaded chunk when passing through a certain gate of current chunk
/// @param m map
/// @param c1 current chunk
/// @param dir gate orientation / type
/// @return created chunk or accessed chunk
chunk* getChunkFrom(map* m, chunk* c1, int dir) {
    if (c1->link[dir] != NULL) {
        return c1->link[dir];
    }
    if (dir != 0) {
        int s = dir < 3 ? 1 : -1;
        chunk* ck = getChunk(m, c1->x + dir % 2 * s, c1->y + s * (dir - 1) % 2);
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
        chunk* ck = getChunk(m, c1->x + x, c1->y + y);
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
    free(ck);
}

/// @brief Free full chunk in the map and itself
/// @param m map
/// @param ck chunk to free
void destroy_chunk(map* m, chunk* ck) {
    purge_chunk(m->hashmap, ck);
}

/// @brief Print the chunk with his coords, pointer, the link pointer and the linked chunks
/// @param ck chunk to lookup
void print_chunk(chunk* ck) {
    printf("CHUNK: %p [", ck);
    printf("x: %d, y: %d, type: %d, element: %p]\n", ck->x, ck->y, ck->type, ck->elements);
    if (false) {
        printf("link (%p): [%p, %p, %p, %p, %p]\n\n", ck->link, ck->link[0], ck->link[1], ck->link[2], ck->link[3], ck->link[4]);
    } else {
        printf("link: [%p, %p, %p, %p, %p]\n\n", ck->link[0], ck->link[1], ck->link[2], ck->link[3], ck->link[4]);
    }
}

/// @brief Print the @map->hashtable and the @map->spawn chunk
/// @param m map
void print_map(map* m) {
    print_hm(m->hashmap);
    print_chunk(m->spawn);
}

//
// int main() {
//     srand(time(NULL));
//     map* m = create_map();
//     print_map(m);
//     chunk* ck = getChunkFrom(m, m->spawn, STARGATE);
//     /*print_map(m);
//     print_chunk(ck);*/
//     printf("(x: %d, y: %d) -> ", ck->x, ck->y);
//     for (int i = 0; i < 50; i++) {
//         int d = rand() % 5;
//         ck = getChunkFrom(m, ck, d);
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