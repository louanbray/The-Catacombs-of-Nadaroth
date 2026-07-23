#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../managers/assets_manager.h"
#include "../utils/logger.h"

#define CHUNK_WIDTH 127
#define CHUNK_HEIGHT 35

char grid[CHUNK_HEIGHT][CHUNK_WIDTH];

typedef struct ItemDef {
    char symbol;
    int type;
    int sprite;
    int size;
    int entity;
    int usable_item;
} ItemDef;

typedef enum IType {
    V_GATE,
    N_GATE,
    S_GATE,
    STAR_GATE,
    WALL,
    EB1,
    EB2,
    ES1,
    ES2,
    EG1,
    EG2,
    EN1,
    EN2,
    EBOSS1,
    CB,
    CS,
    CG,
    CN,
    ITEMS_COUNT,
} IType;

static const ItemDef ITEMS_LOOKUP[ITEMS_COUNT] = {
    // Gates
    [V_GATE] = {'V', 1, 9608, 0, 0, 0},   // GATE, VGATE, COLLAPSE, NOENTITY, NOT_USABLE
    [N_GATE] = {'N', 1, 9600, 0, 0, 0},   // GATE, UGATE, COLLAPSE, NOENTITY, NOT_USABLE
    [S_GATE] = {'S', 1, 9604, 0, 0, 0},   // GATE, DGATE, COLLAPSE, NOENTITY, NOT_USABLE
    [STAR_GATE] = {'P', 2, 0, 0, 13, 0},  // SGATE, ENTITY, COLLAPSE, STAR_GATE, NOT_USABLE

    // Walls
    [WALL] = {'W', 0, 11201, 0, 0, 0},  // WALL, WALL, COLLAPSE, NOENTITY, NOT_USABLE

    // Enemies
    [EB1] = {'A', 4, 9053, 0, 1, 0},      // ENEMY_BRONZE_1
    [EB2] = {'B', 4, 9053, 0, 2, 0},      // ENEMY_BRONZE_2
    [ES1] = {'C', 4, 9053, 0, 3, 0},      // ENEMY_SILVER_1
    [ES2] = {'D', 4, 9053, 0, 4, 0},      // ENEMY_SILVER_2
    [EG1] = {'E', 4, 9053, 0, 5, 0},      // ENEMY_GOLD_1
    [EG2] = {'F', 4, 9053, 0, 6, 0},      // ENEMY_GOLD_2
    [EN1] = {'G', 4, 9053, 0, 7, 0},      // ENEMY_NADINO_1
    [EN2] = {'H', 4, 9053, 0, 8, 0},      // ENEMY_NADINO_2
    [EBOSS1] = {'I', 4, 9053, 0, 14, 0},  // ENEMY_BOSS

    // Lootables
    [CB] = {'0', 5, 0, 0, 9, 0},   // BRONZE_CHEST
    [CS] = {'1', 5, 0, 0, 10, 0},  // SILVER_CHEST
    [CG] = {'2', 5, 0, 0, 11, 0},  // GOLD_CHEST
    [CN] = {'3', 5, 0, 0, 12, 0}   // NADINO_CHEST
};

typedef struct DifficultyChunkStats {
    int nb_fil_min, nb_fil_max;
    int fil_smin, fil_smax;
    int nb_iso_min, nb_iso_max;
    int nb_items_min[ITEMS_COUNT];
    int nb_items_max[ITEMS_COUNT];
} DifficultyChunkStats;

static const DifficultyChunkStats DIFF_STATS_LOOKUP[CHUNK_TYPE_COUNT] = {
    [CHUNK_DEFAULT] = {
        .nb_fil_min = 35,
        .nb_fil_max = 45,
        .fil_smin = 10,
        .fil_smax = 50,
        .nb_iso_min = 100,
        .nb_iso_max = 120,
        .nb_items_min = {0},
        .nb_items_max = {0},
    },
    [CHUNK_DEFAULT2] = {
        .nb_fil_min = 55,
        .nb_fil_max = 65,
        .fil_smin = 5,
        .fil_smax = 35,
        .nb_iso_min = 90,
        .nb_iso_max = 110,
        .nb_items_min = {0},
        .nb_items_max = {0},
    },
    [CHUNK_TREASURE_ROOM] = {
        .nb_fil_min = 60,
        .nb_fil_max = 80,
        .fil_smin = 10,
        .fil_smax = 25,
        .nb_iso_min = 180,
        .nb_iso_max = 220,
        .nb_items_min = {[CB] = 2, [CS] = 1},
        .nb_items_max = {[CB] = 4, [CS] = 2},
    },
    [CHUNK_WAITING_ROOM] = {
        .nb_fil_min = 16,
        .nb_fil_max = 20,
        .fil_smin = 25,
        .fil_smax = 35,
        .nb_iso_min = 120,
        .nb_iso_max = 150,
        .nb_items_min = {0},
        .nb_items_max = {0},
    },
    [CHUNK_RANDOM_EASY] = {
        .nb_fil_min = 45,
        .nb_fil_max = 55,
        .fil_smin = 8,
        .fil_smax = 30,
        .nb_iso_min = 160,
        .nb_iso_max = 200,
        .nb_items_min = {[EB1] = 3, [EB2] = 3, [CB] = 0, [CS] = 1},
        .nb_items_max = {[EB1] = 4, [EB2] = 3, [CB] = 1, [CS] = 2},
    },
    [CHUNK_RANDOM_MEDIUM] = {
        .nb_fil_min = 45,
        .nb_fil_max = 55,
        .fil_smin = 8,
        .fil_smax = 30,
        .nb_iso_min = 160,
        .nb_iso_max = 200,
        .nb_items_min = {[EB1] = 2, [EB2] = 2, [ES1] = 1, [ES2] = 1, [CB] = 1, [CS] = 1, [CG] = 0},
        .nb_items_max = {[EB1] = 3, [EB2] = 3, [ES1] = 2, [ES2] = 2, [CB] = 1, [CS] = 2, [CG] = 1},
    },
    [CHUNK_RANDOM_HARD] = {
        .nb_fil_min = 50,
        .nb_fil_max = 55,
        .fil_smin = 8,
        .fil_smax = 30,
        .nb_iso_min = 140,
        .nb_iso_max = 180,
        .nb_items_min = {[ES1] = 1, [ES2] = 1, [EG1] = 3, [EG2] = 1, [CS] = 1, [CG] = 2, [CN] = 0},
        .nb_items_max = {[ES1] = 2, [ES2] = 2, [EG1] = 5, [EG2] = 2, [CS] = 2, [CG] = 3, [CN] = 1},
    },
    [CHUNK_RANDOM_NADINHARD] = {
        .nb_fil_min = 50,
        .nb_fil_max = 55,
        .fil_smin = 8,
        .fil_smax = 25,
        .nb_iso_min = 180,
        .nb_iso_max = 220,
        .nb_items_min = {[ES2] = 0, [EG1] = 1, [EG2] = 1, [EN1] = 2, [EN2] = 1, [CS] = 1, [CG] = 3, [CN] = 1},
        .nb_items_max = {[ES2] = 1, [EG1] = 2, [EG2] = 2, [EN1] = 4, [EN2] = 2, [CS] = 2, [CG] = 4, [CN] = 1},
    },
};

#define DSTATS_COUNT (sizeof(DIFF_STATS_LOOKUP) / sizeof(DifficultyChunkStats))

const ItemDef* get_item_def(char symbol) {
    for (size_t i = 0; i < ITEMS_COUNT; i++) {
        if (ITEMS_LOOKUP[i].symbol == symbol) {
            return &ITEMS_LOOKUP[i];
        }
    }
    return NULL;
}

typedef struct {
    int x;
    int y;
} Point;

bool is_stargate_cell(int x, int y) {
    char c = grid[y][x];
    if (c == 'P') return true;

    if (c == '#') {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -2; dx <= 2; dx += 2) {
                int cx = x + dx;
                int cy = y + dy;
                if (cx >= 0 && cx < CHUNK_WIDTH && cy >= 0 && cy < CHUNK_HEIGHT) {
                    if (grid[cy][cx] == 'P') return true;
                }
            }
        }
    }
    return false;
}

bool is_walkable(int x, int y) {
    if (x < 0 || x >= CHUNK_WIDTH || y < 0 || y >= CHUNK_HEIGHT) return false;
    char c = grid[y][x];
    return (c == ' ' || c == 'N' || c == 'S' || c == 'V' || c == 'P' || is_stargate_cell(x, y));
}

bool check_chunk_connectivity() {
    bool visited[CHUNK_HEIGHT][CHUNK_WIDTH] = {false};
    Point queue[CHUNK_HEIGHT * CHUNK_WIDTH];
    int head = 0, tail = 0;

    int start_x = 62, start_y = 17;
    queue[tail++] = (Point){start_x, start_y};
    visited[start_y][start_x] = true;

    int dx[] = {-2, 2, 0, 0};
    int dy[] = {0, 0, -1, 1};

    while (head < tail) {
        Point p = queue[head++];

        for (int i = 0; i < 4; i++) {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];

            if (nx >= 0 && nx < CHUNK_WIDTH && ny >= 0 && ny < CHUNK_HEIGHT) {
                if (!visited[ny][nx] && is_walkable(nx, ny)) {
                    visited[ny][nx] = true;
                    queue[tail++] = (Point){nx, ny};
                }
            }
        }
    }

    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int x = 0; x < CHUNK_WIDTH; x += 2) {
            char cell = grid[y][x];

            if ((cell == 'N' || cell == 'S' || cell == 'V' || cell == 'P') && !visited[y][x]) {
                return false;  // The gate is blocked by walls
            }
        }
    }

    return true;  // All gate accessible
}

bool is_center_3x3(int x, int y) {
    if (y >= 16 && y <= 18 && x >= 60 && x <= 64) {
        return true;
    }
    return false;
}

void init_grid() {
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int x = 0; x < CHUNK_WIDTH; x++) {
            grid[y][x] = (x % 2 == 0) ? ' ' : '*';
        }
    }
}

void apply_gates() {
    for (int x = 62; x <= 67; x += 1) {
        grid[0][x] = 'N';
        grid[CHUNK_HEIGHT - 1][x] = 'S';
    }

    // West Gate
    grid[16][0] = 'V';
    grid[17][0] = 'V';
    grid[18][0] = 'V';
    // East Gate
    grid[16][126] = 'V';
    grid[17][126] = 'V';
    grid[18][126] = 'V';
}

void place_filament(int length) {
    int r = rand() % CHUNK_HEIGHT;
    int c = rand() % CHUNK_WIDTH;

    for (int step = 0; step < length; step++) {
        // grid[r][c] = ' ' only on even cols
        if (grid[r][c] == ' ' && !is_center_3x3(c, r)) {
            grid[r][c] = 'W';
        }

        int dir = rand() % 4;
        int dr = 0, dc = 0;
        if (dir == 0)
            dr = 1;  // Down
        else if (dir == 1)
            dr = -1;  // Up
        else if (dir == 2)
            dc = 1;  // Right
        else if (dir == 3)
            dc = -1;  // Left

        r += dr;
        if (r < 0) r = 0;
        if (r >= CHUNK_HEIGHT) r = CHUNK_HEIGHT - 1;

        c += dc;
        if (c < 0) c = 0;
        if (c >= CHUNK_WIDTH) c = CHUNK_WIDTH - 1;
    }
}

void generate_decor(int nb_fil, int fil_smin, int fil_smax, int nb_iso) {
    for (int i = 0; i < nb_fil; i++) {
        int length = RAND_RANGE(fil_smin, fil_smax);
        place_filament(length);
    }

    for (int i = 0; i < nb_iso; i++) {
        int r = rand() % CHUNK_HEIGHT;
        int c = rand() % CHUNK_WIDTH;

        if (grid[r][c] == ' ' && !is_center_3x3(c, r)) {
            grid[r][c] = 'W';
        }
    }
}

bool is_valid_for_3x3(int center_x, int center_y) {
    if (center_x < 2 || center_x >= CHUNK_WIDTH - 2 ||
        center_y < 1 || center_y >= CHUNK_HEIGHT - 1) {
        return false;
    }

    for (int dx = -2; dx <= 2; dx += 2) {
        for (int dy = -1; dy <= 1; dy++) {
            int check_x = center_x + dx;
            int check_y = center_y + dy;

            if (is_center_3x3(check_x, check_y)) {
                return false;
            }

            char cell = grid[check_y][check_x];
            if (cell != ' ' && cell != '*') {
                return false;
            }
        }
    }
    return true;
}

void place_3x3_entity(int center_x, int center_y, char enemy_type) {
    for (int dx = -2; dx <= 2; dx += 2) {
        for (int dy = -1; dy <= 1; dy++) {
            int x = center_x + dx;
            int y = center_y + dy;

            if (dx == 0 && dy == 0) {
                grid[y][x] = enemy_type;
            } else {
                grid[y][x] = '#';  // Prevent overlapping
            }
        }
    }
}

bool populate(int target_count, char item_type) {
    int placed = 0, attempts = 0;
    while (placed < target_count && attempts < 1000) {
        attempts++;
        int x = (rand() % (CHUNK_WIDTH / 2)) * 2;
        int y = rand() % CHUNK_HEIGHT;

        if (is_valid_for_3x3(x, y)) {
            place_3x3_entity(x, y, item_type);
            placed++;
        }
    }
    return placed == target_count;
}

bool init(ChunkType type) {
    DifficultyChunkStats d = DIFF_STATS_LOOKUP[type];
    int attempts = 0;
    bool sg_placed = false;
    do {
        attempts++;

        init_grid();
        apply_gates();
        generate_decor(RAND_RANGE(d.nb_fil_min, d.nb_fil_max), d.fil_smin, d.fil_smax, RAND_RANGE(d.nb_iso_min, d.nb_iso_max));

        sg_placed = populate(1, 'P');
        for (IType i = V_GATE; i < ITEMS_COUNT; i++) {
            int nb = RAND_RANGE(d.nb_items_min[i], d.nb_items_max[i]);
            if (nb) populate(nb, ITEMS_LOOKUP[i].symbol);
        }
    } while ((!check_chunk_connectivity() || !sg_placed) && attempts < 100);
    if (attempts >= 100) {
        LOG_ERROR("Couldn't generate a valid chunk after 100 tries");
        LOG_ERROR("Ooga Booga Badluck You Do Have :)");
        return false;
    }
    return true;
}

void export_chunk_to_file(const char* filename, ChunkType type) {
    if (!init(type)) return;
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        LOG_ERROR("Error opening file");
        return;
    }

    char chunk_copy[CHUNK_HEIGHT][CHUNK_WIDTH];
    for (int i = 0; i < CHUNK_HEIGHT; i++) {
        for (int j = 0; j < CHUNK_WIDTH; j++) {
            chunk_copy[i][j] = grid[i][j];
        }
    }

    for (int i = 0; i < CHUNK_HEIGHT; i++) {
        int row_repeat = 1;
        int col_repeat = 1;

        for (int j = 0; j < CHUNK_WIDTH; j++) {
            char cell = chunk_copy[i][j];

            const ItemDef* item = get_item_def(cell);
            if (!item) continue;

            int size = item->size;

            if (j < CHUNK_WIDTH - 1 - size && cell == chunk_copy[i][j + 1 + size]) {
                row_repeat++;
            } else {
                if (i < CHUNK_HEIGHT - 1 && row_repeat == 1) {
                    for (int k = 0; k < CHUNK_HEIGHT - 1 - i; k++) {
                        if (cell == chunk_copy[i + 1 + k][j]) {
                            col_repeat++;
                            chunk_copy[i + 1 + k][j] = ' ';
                        } else {
                            break;
                        }
                    }
                }

                int game_x = j - 63;
                int game_y = -i + 17;

                fprintf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                        game_x,
                        game_y,
                        item->type,
                        item->sprite,
                        row_repeat,
                        item->size,
                        col_repeat,
                        item->entity,
                        item->usable_item);

                row_repeat = 1;
                col_repeat = 1;
            }
        }
    }

    fclose(file);
}

ChunkAssetFile* generate_chunk_asset_file(ChunkType type) {
    if (!init(type)) return NULL;
    ChunkAssetFile* chunk = malloc(sizeof(ChunkAssetFile));
    if (!chunk) return NULL;

    chunk->items = NULL;
    chunk->item_count = 0;
    chunk->can_free = true;

    char chunk_copy[CHUNK_HEIGHT][CHUNK_WIDTH];
    for (int i = 0; i < CHUNK_HEIGHT; i++) {
        for (int j = 0; j < CHUNK_WIDTH; j++) {
            chunk_copy[i][j] = grid[i][j];
        }
    }

    for (int i = 0; i < CHUNK_HEIGHT; i++) {
        int row_repeat = 1;
        int col_repeat = 1;

        for (int j = 0; j < CHUNK_WIDTH; j++) {
            char cell = chunk_copy[i][j];

            const ItemDef* item = get_item_def(cell);
            if (!item) continue;

            int size = item->size;
            if (j < CHUNK_WIDTH - 1 - size && cell == chunk_copy[i][j + 1 + size]) {
                row_repeat++;
            } else {
                if (i < CHUNK_HEIGHT - 1 && row_repeat == 1) {
                    for (int k = 0; k < CHUNK_HEIGHT - 1 - i; k++) {
                        if (cell == chunk_copy[i + 1 + k][j]) {
                            col_repeat++;
                            chunk_copy[i + 1 + k][j] = ' ';
                        } else {
                            break;
                        }
                    }
                }

                int game_x = j - 63;
                int game_y = -i + 17;

                chunk->items = realloc(chunk->items, sizeof(ChunkItem) * (chunk->item_count + 1));
                ChunkItem* it = &chunk->items[chunk->item_count++];
                it->x = game_x;
                it->y = game_y;
                it->type = item->type;
                it->display = item->sprite;
                it->row_repeat = row_repeat;
                it->size = item->size;
                it->col_repeat = col_repeat;
                it->entity_type = item->entity;
                it->usable_item = item->usable_item;

                row_repeat = 1;
                col_repeat = 1;
            }
        }
    }

    return chunk;
}

// int main() {
//     srand(time(NULL));
//     export_chunk_to_file("../../assets/chunks/generated_level.dodjo", CHUNK_WAITING_ROOM);
//     return 0;
// }