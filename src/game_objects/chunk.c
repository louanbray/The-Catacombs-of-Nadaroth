#include "chunk.h"

#include <stdlib.h>
#include <string.h>

#include "../display/render.h"
#include "../utils/game_status.h"
#include "item.h"

/// @brief Private chunk structure definition
typedef struct chunk {
    chunk_link link;
    int x, spawn_x;
    int y, spawn_y;
    ChunkType type;
    dynarray* elements;
    dynarray* enemies;
    hm* hashmap;
    chunk_arena arena;
    uint64_t wall_mask[CHUNK_MATRIX_HEIGHT][2];
    wall_entry* sparse_walls;
    uint16_t wall_count;
    uint16_t wall_capacity;
} chunk;

void* chunk_arena_alloc(chunk_arena* arena, size_t size) {
    if (!arena) return malloc(size);

    size = (size + 7) & ~((size_t)7);

    if (arena->current == NULL || arena->current->offset + size > arena->current->capacity) {
        size_t block_cap = size > 4096 ? size : 4096;
        arena_block* block = (arena_block*)malloc(sizeof(arena_block) + block_cap);
        if (!block) return NULL;
        block->next = NULL;
        block->capacity = block_cap;
        block->offset = 0;

        if (arena->current != NULL) {
            arena->current->next = block;
        } else {
            arena->head = block;
        }
        arena->current = block;
    }

    void* ptr = arena->current->data + arena->current->offset;
    arena->current->offset += size;
    return ptr;
}

void chunk_arena_free_all(chunk_arena* arena) {
    if (!arena) return;
    arena_block* curr = arena->head;
    while (curr != NULL) {
        arena_block* next = curr->next;
        free(curr);
        curr = next;
    }
    arena->head = NULL;
    arena->current = NULL;
}

chunk_arena* get_chunk_arena(chunk* ck) {
    return ck ? &ck->arena : NULL;
}

static inline bool chunk_coords_to_matrix(int x, int y, int* out_row, int* out_col) {
    int col = x + 64;
    int row = 17 - y;
    if (col < 0 || col >= CHUNK_MATRIX_WIDTH || row < 0 || row >= CHUNK_MATRIX_HEIGHT) {
        return false;
    }
    *out_col = col;
    *out_row = row;
    return true;
}

bool chunk_has_wall(chunk* ck, int x, int y) {
    if (!ck) return false;
    int row, col;
    if (chunk_coords_to_matrix(x, y, &row, &col)) {
        int word_idx = col / 64;
        int bit_idx = col % 64;
        return (ck->wall_mask[row][word_idx] & (1ULL << bit_idx)) != 0;
    }
    return false;
}

void chunk_set_wall(chunk* ck, int x, int y, int display, Color color) {
    if (!ck) return;
    int row, col;
    if (!chunk_coords_to_matrix(x, y, &row, &col)) return;

    int word_idx = col / 64;
    int bit_idx = col % 64;
    ck->wall_mask[row][word_idx] |= (1ULL << bit_idx);

    for (uint16_t i = 0; i < ck->wall_count; i++) {
        if (ck->sparse_walls[i].row == (uint8_t)row && ck->sparse_walls[i].col == (uint8_t)col) {
            ck->sparse_walls[i].display = (uint16_t)display;
            ck->sparse_walls[i].color = (uint8_t)color;
            return;
        }
    }

    if (ck->wall_count == ck->wall_capacity) {
        uint16_t new_cap = ck->wall_capacity == 0 ? 64 : ck->wall_capacity * 2;
        wall_entry* new_array = (wall_entry*)realloc(ck->sparse_walls, new_cap * sizeof(wall_entry));
        if (!new_array) return;
        ck->sparse_walls = new_array;
        ck->wall_capacity = new_cap;
    }

    ck->sparse_walls[ck->wall_count].row = (uint8_t)row;
    ck->sparse_walls[ck->wall_count].col = (uint8_t)col;
    ck->sparse_walls[ck->wall_count].display = (uint16_t)display;
    ck->sparse_walls[ck->wall_count].color = (uint8_t)color;
    ck->wall_count++;
}

void chunk_clear_wall(chunk* ck, int x, int y) {
    if (!ck) return;
    int row, col;
    if (!chunk_coords_to_matrix(x, y, &row, &col)) return;

    int word_idx = col / 64;
    int bit_idx = col % 64;
    ck->wall_mask[row][word_idx] &= ~(1ULL << bit_idx);

    for (uint16_t i = 0; i < ck->wall_count; i++) {
        if (ck->sparse_walls[i].row == (uint8_t)row && ck->sparse_walls[i].col == (uint8_t)col) {
            ck->sparse_walls[i] = ck->sparse_walls[ck->wall_count - 1];
            ck->wall_count--;
            return;
        }
    }
}

int chunk_get_wall_display(chunk* ck, int x, int y) {
    if (!chunk_has_wall(ck, x, y)) return 0;
    int row, col;
    chunk_coords_to_matrix(x, y, &row, &col);
    for (uint16_t i = 0; i < ck->wall_count; i++) {
        if (ck->sparse_walls[i].row == (uint8_t)row && ck->sparse_walls[i].col == (uint8_t)col) {
            return (int)ck->sparse_walls[i].display;
        }
    }
    return 0;
}

Color chunk_get_wall_color(chunk* ck, int x, int y) {
    if (!chunk_has_wall(ck, x, y)) return COLOR_DEFAULT;
    int row, col;
    chunk_coords_to_matrix(x, y, &row, &col);
    for (uint16_t i = 0; i < ck->wall_count; i++) {
        if (ck->sparse_walls[i].row == (uint8_t)row && ck->sparse_walls[i].col == (uint8_t)col) {
            return (Color)ck->sparse_walls[i].color;
        }
    }
    return COLOR_DEFAULT;
}

void chunk_render_walls(chunk* ck, void* board_ptr) {
    if (!ck || !board_ptr) return;
    board b = (board)board_ptr;
    for (uint16_t i = 0; i < ck->wall_count; i++) {
        wall_entry* w = &ck->sparse_walls[i];
        int word_idx = w->col / 64;
        int bit_idx = w->col % 64;
        if (ck->wall_mask[w->row][word_idx] & (1ULL << bit_idx)) {
            int game_x = w->col - 64;
            int game_y = 17 - w->row;
            render_char_colored(b, game_x, game_y, w->display, w->color);
        }
    }
}

/// @brief Create an empty set of 5 chunk links
static chunk_link create_link() {
    return calloc(5, sizeof(chunk*));
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

int get_chunk_spawn_x(chunk* c) {
    return c->spawn_x;
}

int get_chunk_spawn_y(chunk* c) {
    return c->spawn_y;
}

void set_chunk_type(chunk* ck, ChunkType type) {
    ck->type = type;
}

dynarray* get_chunk_enemies(chunk* ck) {
    return ck->enemies;
}

ChunkType get_chunk_type(chunk* ck) {
    return ck->type;
}

chunk_link get_chunk_links(chunk* ck) {
    return ck->link;
}

chunk* get_chunk_link_at(chunk* ck, int dir) {
    return ck->link[dir];
}

uint16_t get_chunk_wall_count(chunk* ck) {
    return ck ? ck->wall_count : 0;
}

wall_entry* get_chunk_sparse_walls(chunk* ck) {
    return ck ? ck->sparse_walls : NULL;
}

void* get_chunk_wall_mask(chunk* ck) {
    return ck ? ck->wall_mask : NULL;
}

void set_chunk_walls_data(chunk* ck, uint64_t wall_mask[CHUNK_MATRIX_HEIGHT][2], uint16_t count, wall_entry* walls) {
    if (!ck) return;
    if (wall_mask) {
        memcpy(ck->wall_mask, wall_mask, sizeof(ck->wall_mask));
    } else {
        memset(ck->wall_mask, 0, sizeof(ck->wall_mask));
    }
    if (ck->sparse_walls) {
        free(ck->sparse_walls);
    }
    ck->wall_count = count;
    ck->wall_capacity = count;
    ck->sparse_walls = walls;
}

void set_chunk_link(chunk* ck, int dir, chunk* target) {
    ck->link[dir] = target;
}

void chunk_append_element(chunk* ck, item* it) {
    append(ck->elements, it);
}

void chunk_register_item(chunk* ck, item* it) {
    set_hm_arena(ck->hashmap, get_item_x(it), get_item_y(it), it, &ck->arena);
}

void chunk_append_enemy(chunk* ck, item* it) {
    append(ck->enemies, it);
}

int chunk_element_count(chunk* ck) {
    return len_dyn(ck->elements);
}

item* chunk_get_element(chunk* ck, int i) {
    return (item*)get_dyn(ck->elements, i);
}

chunk* create_chunk(int x, int y) {
    chunk* ck = malloc(sizeof(chunk));
    ck->arena = (chunk_arena){0};
    ck->link = create_link();
    ck->x = x;
    ck->y = y;
    ck->type = CHUNK_SPAWN;
    ck->spawn_x = 1;
    ck->spawn_y = 0;
    ck->elements = create_dyn();
    ck->enemies = create_dyn();
    ck->hashmap = create_hashmap();
    ck->sparse_walls = NULL;
    ck->wall_count = 0;
    ck->wall_capacity = 0;
    memset(ck->wall_mask, 0, sizeof(ck->wall_mask));
    return ck;
}

chunk* generate_chunk(int x, int y) {
    chunk* c = create_chunk(x, y);
    decorate(c, x, y);
    return c;
}

chunk* create_chunk_raw(int x, int y, int spawn_x, int spawn_y, ChunkType type) {
    chunk* ck = malloc(sizeof(chunk));
    ck->arena = (chunk_arena){0};
    ck->link = create_link();
    ck->x = x;
    ck->y = y;
    ck->spawn_x = spawn_x;
    ck->spawn_y = spawn_y;
    ck->type = type;
    ck->elements = create_dyn();
    ck->enemies = create_dyn();
    ck->hashmap = create_hashmap();
    ck->sparse_walls = NULL;
    ck->wall_count = 0;
    ck->wall_capacity = 0;
    memset(ck->wall_mask, 0, sizeof(ck->wall_mask));
    return ck;
}

void reset_chunk_internals(chunk* ck, int spawn_x, int spawn_y, ChunkType type) {
    free_dyn(ck->elements);
    free_dyn_no_item(ck->enemies);
    free_hm(ck->hashmap);
    chunk_arena_free_all(&ck->arena);
    if (ck->sparse_walls) {
        free(ck->sparse_walls);
        ck->sparse_walls = NULL;
    }
    ck->wall_count = 0;
    ck->wall_capacity = 0;
    memset(ck->wall_mask, 0, sizeof(ck->wall_mask));
    ck->elements = create_dyn();
    ck->enemies = create_dyn();
    ck->hashmap = create_hashmap();
    ck->spawn_x = spawn_x;
    ck->spawn_y = spawn_y;
    ck->type = type;
}

void destroy_chunk_full(chunk* ck) {
    if (ck == NULL) return;
    free(ck->link);
    if (ck->sparse_walls) free(ck->sparse_walls);
    free_dyn(ck->elements);
    free_dyn_no_item(ck->enemies);
    free_hm(ck->hashmap);
    chunk_arena_free_all(&ck->arena);
    free(ck);
}

/// @brief Copy the content of the items dynarray to put it into the hashmap
/// @param c chunk
void fill_chunk_hm_from_dyn(chunk* c) {
    dynarray* dyn = c->elements;
    hm* hashmap = c->hashmap;
    int len = len_dyn(dyn);
    for (int i = 0; i < len; i++) {
        item* e = get_dyn(dyn, i);
        if (e != NULL) {
            set_hm_arena(hashmap, get_item_x(e), get_item_y(e), e, &c->arena);
        }
    }
}

/// @brief Add all elements of the chunk depending of the type
/// @param c chunk
/// @param type type
void fill_furniture(chunk* c, ChunkType type) {
    parse_chunk(c, c->elements, type);
    fill_chunk_hm_from_dyn(c);
}

void decorate(chunk* c, int x, int y) {
    int type = is_debug_mode() ? CHUNK_SINGLE : CHUNK_SPAWN;
    int spawn_x = 1;  //! TO CENTER THE PLAYER
    int spawn_y = is_debug_mode() ? -5 : 0;
    if (x != 0 || y != 0) type = RAND_RANGE(CHUNK_SPAWN + 1, CHUNK_TYPE_COUNT - 1);
    c->type = type;
    c->spawn_x = spawn_x;
    c->spawn_y = spawn_y;
    fill_furniture(c, type);
}

void remove_item(chunk* ck, item* i) {
    if (i == NULL) return;
    hm* h = ck->hashmap;
    dynarray* d = ck->elements;
    purge_hm(h, get_item_x(i), get_item_y(i));
    set_dyn(d, get_item_index(i), NULL);
}

void add_item(chunk* ck, item* i) {
    if (i == NULL) return;
    hm* h = ck->hashmap;
    dynarray* d = ck->elements;
    set_item_index(i, len_dyn(d));
    append(d, i);
    set_hm_arena(h, get_item_x(i), get_item_y(i), i, &ck->arena);
}