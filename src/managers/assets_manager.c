#include "assets_manager.h"

#include <stdio.h>
#include <string.h>

#include "../scripts/chunk_generation.h"
#include "../utils/logger.h"

static AssetManager* asset_manager = NULL;

/**
 * @brief Loads an entity asset file and parses its contents.
 *
 * This function opens the specified file, reads its contents, and parses the
 * specifications and entity parts. The specifications are expected to be in
 * the first line of the file, enclosed in square brackets and separated by
 * commas. The entity parts are expected to be in the subsequent lines, each
 * line containing five integers separated by commas.
 *
 * @param filename The path to the entity asset file to be loaded.
 * @return A pointer to the loaded EntityAssetFile structure, or NULL if an
 * error occurred.
 */
static EntityAssetFile* load_entity_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    EntityAssetFile* entity_file = malloc(sizeof(EntityAssetFile));
    if (!entity_file) return NULL;

    entity_file->parts = NULL;
    entity_file->count = 0;

    // Parse the specs line
    char specs_line[256];
    if (fgets(specs_line, sizeof(specs_line), file) != NULL) {
        char* ending = strchr(specs_line, ']');
        if (specs_line[0] == '[' && ending != NULL) {
            ending[0] = '\0';
            char* start = specs_line + 1;

            // Parse dynamic specs
            size_t count = 0;
            int* specs = NULL;

            char* token = strtok(start, ",");
            while (token != NULL) {
                specs = realloc(specs, (count + 1) * sizeof(int));

                if (!specs) {
                    fclose(file);
                    free(entity_file);
                    return NULL;
                }

                specs[count++] = atoi(token);
                token = strtok(NULL, ",");
            }

            entity_file->specs.specs = specs;
            entity_file->specs.spec_count = count;
        } else {
            fclose(file);
            free(entity_file);
            return NULL;
        }
    }

    // Parse entity parts
    EntityPart part;
    while (fscanf(file, "%d,%d,%d,%d,%d", &part.x, &part.y, &part.display, &part.row_repeat, &part.col_repeat) == 5) {
        entity_file->parts = realloc(entity_file->parts, (entity_file->count + 1) * sizeof(EntityPart));

        if (!entity_file->parts) {
            fclose(file);
            free(entity_file->specs.specs);
            free(entity_file);
            return NULL;
        }
        entity_file->parts[entity_file->count++] = part;
    }

    fclose(file);
    return entity_file;
}

/**
 * @brief Loads a chunk file and parses its contents into a ChunkAssetFile structure.
 *
 * This function opens a file specified by the filename, reads its contents, and
 * populates a ChunkAssetFile structure with the parsed data. Each line in the file
 * is expected to contain eight comma-separated integers representing the properties
 * of a ChunkItem.
 *
 * @param filename The path to the file to be loaded.
 * @return A pointer to a ChunkAssetFile structure containing the parsed data, or NULL
 *         if the file could not be opened or memory allocation failed.
 */
static ChunkAssetFile* load_chunk_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    ChunkAssetFile* chunk = malloc(sizeof(ChunkAssetFile));
    if (!chunk) return NULL;

    chunk->items = NULL;
    chunk->item_count = 0;

    int x, y, type, display, row_repeat, size, col_repeat, entity_type, usable_item;
    while (fscanf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d", &x, &y, &type, &display, &row_repeat, &size, &col_repeat, &entity_type, &usable_item) == 9) {
        chunk->items = realloc(chunk->items, sizeof(ChunkItem) * (chunk->item_count + 1));
        ChunkItem* item = &chunk->items[chunk->item_count++];
        item->x = x;
        item->y = y;
        item->type = type;
        item->display = display;
        item->row_repeat = row_repeat;
        item->size = size;
        item->col_repeat = col_repeat;
        item->entity_type = entity_type;
        item->usable_item = usable_item;
    }

    fclose(file);
    return chunk;
}

static UsableItemAssetFile* load_usable_item_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    UsableItemAssetFile* usable_item = malloc(sizeof(UsableItemAssetFile));
    if (!usable_item) return NULL;

    // Parse the specs line
    char specs_line[256];
    if (fgets(specs_line, sizeof(specs_line), file) != NULL) {
        char* ending = strchr(specs_line, ']');
        if (specs_line[0] == '[' && ending != NULL) {
            ending[0] = '\0';
            char* start = specs_line + 1;

            // Parse dynamic specs
            size_t count = 0;
            int* specs = NULL;

            char* token = strtok(start, ",");
            while (token != NULL) {
                specs = realloc(specs, (count + 1) * sizeof(int));

                if (!specs) {
                    fclose(file);
                    free(usable_item);
                    return NULL;
                }

                specs[count++] = atoi(token);
                token = strtok(NULL, ",");
            }

            usable_item->specs.specs = specs;
            usable_item->specs.spec_count = count;
        } else {
            fclose(file);
            free(usable_item);
            return NULL;
        }
    } else {
        fclose(file);
        free(usable_item);
        return NULL;
    }

    // Parse the title (second line)
    char title_line[256];
    if (fgets(title_line, sizeof(title_line), file) != NULL) {
        title_line[strcspn(title_line, "\n")] = '\0';  // Remove newline character
        usable_item->title = strdup(title_line);
        if (!usable_item->title) {
            fclose(file);
            free(usable_item->specs.specs);
            free(usable_item);
            return NULL;
        }
    } else {
        fclose(file);
        free(usable_item->specs.specs);
        free(usable_item);
        return NULL;
    }

    // Parse the description (remaining lines)
    char description_buffer[1024] = {0};
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        strcat(description_buffer, line);
    }

    // Allocate and copy the description
    usable_item->description = strdup(description_buffer);
    if (!usable_item->description) {
        fclose(file);
        free(usable_item->title);
        free(usable_item->specs.specs);
        free(usable_item);
        return NULL;
    }

    fclose(file);
    return usable_item;
}

/**
 * @brief Creates and initializes an AssetManager structure.
 *
 * This function allocates memory for an AssetManager structure and initializes its
 * entity and chunk arrays to NULL. The number of entities and chunks is determined
 * by the ENTITY_TYPE_COUNT and CHUNK_TYPE_COUNT constants, respectively.
 *
 * @return A pointer to an initialized AssetManager structure, or NULL if memory
 *         allocation failed.
 */
AssetManager* create_asset_manager() {
    AssetManager* manager = malloc(sizeof(AssetManager));
    if (!manager) return NULL;

    for (int i = 0; i < ENTITY_TYPE_COUNT; i++) {
        manager->entities[i] = NULL;
    }
    for (int i = 0; i < CHUNK_TYPE_COUNT; i++) {
        manager->chunks[i] = NULL;
    }
    for (int i = 0; i < USABLE_ITEM_COUNT; i++) {
        manager->usable_items[i] = NULL;
    }

    return manager;
}

bool add_entity_file(const char* filename, EntityType type) {
    if (type < 0 || type >= ENTITY_TYPE_COUNT) return NULL;

    EntityAssetFile* entity = load_entity_file(filename);
    if (!entity) return false;

    asset_manager->entities[type] = entity;
    return true;
}

bool add_chunk_file(const char* filename, ChunkType type) {
    if (type < 0 || type >= CHUNK_TYPE_COUNT) return NULL;

    ChunkAssetFile* chunk = load_chunk_file(filename);
    if (!chunk) return false;

    asset_manager->chunks[type] = chunk;
    return true;
}

bool add_generated_chunk(const char* filename, ChunkType type) {
    if (type < 0 || type >= CHUNK_TYPE_COUNT) return NULL;

    ChunkAssetFile* chunk = generate_chunk_asset_file(type);
    if (!chunk) return false;

    asset_manager->chunks[type] = chunk;
    return true;
}

bool add_usable_item_file(const char* filename, UsableItem type) {
    if (type < 0 || type >= USABLE_ITEM_COUNT) return NULL;

    UsableItemAssetFile* usable_item = load_usable_item_file(filename);
    if (!usable_item) return false;

    asset_manager->usable_items[type] = usable_item;
    return true;
}

EntityAssetFile* get_entity_file(EntityType type) {
    if (type < 0 || type >= ENTITY_TYPE_COUNT) return NULL;

    return asset_manager->entities[type];
}

ChunkAssetFile* get_chunk_file(ChunkType type) {
    if (type < 0 || type >= CHUNK_TYPE_COUNT) return NULL;

    return asset_manager->chunks[type];
}

UsableItemAssetFile* get_usable_item_file(UsableItem type) {
    if (type < 0 || type >= USABLE_ITEM_COUNT) return NULL;

    return asset_manager->usable_items[type];
}

void destroy_asset_manager() {
    if (!asset_manager) return;

    for (int i = 0; i < ENTITY_TYPE_COUNT; i++) {
        if (!asset_manager->entities[i]) continue;
        free(asset_manager->entities[i]->specs.specs);
        free(asset_manager->entities[i]->parts);
        free(asset_manager->entities[i]);
    }

    for (int i = 0; i < CHUNK_TYPE_COUNT; i++) {
        if (!asset_manager->chunks[i]) continue;
        free(asset_manager->chunks[i]->items);
        free(asset_manager->chunks[i]);
    }

    for (int i = 0; i < USABLE_ITEM_COUNT; i++) {
        if (!asset_manager->usable_items[i]) continue;
        free(asset_manager->usable_items[i]->title);
        free(asset_manager->usable_items[i]->description);
        free(asset_manager->usable_items[i]->specs.specs);
        free(asset_manager->usable_items[i]);
    }
    free(asset_manager);
    asset_manager = NULL;
}

void init_assets_system() {
    asset_manager = create_asset_manager();
    if (!asset_manager) {
        LOG_ERROR("Failed to create asset manager\n");
        exit(1);
    }

    // Load entity files
    // ---- Basic enemies ----
    add_entity_file("assets/entities/data/enemy_bronze_1.dodjo", ENTITY_ENEMY_BRONZE_1);
    add_entity_file("assets/entities/data/enemy_bronze_2.dodjo", ENTITY_ENEMY_BRONZE_2);
    add_entity_file("assets/entities/data/enemy_silver_1.dodjo", ENTITY_ENEMY_SILVER_1);
    add_entity_file("assets/entities/data/enemy_silver_2.dodjo", ENTITY_ENEMY_SILVER_2);
    add_entity_file("assets/entities/data/enemy_gold_1.dodjo", ENTITY_ENEMY_GOLD_1);
    add_entity_file("assets/entities/data/enemy_gold_2.dodjo", ENTITY_ENEMY_GOLD_2);
    add_entity_file("assets/entities/data/enemy_nadino_1.dodjo", ENTITY_ENEMY_NADINO_1);
    add_entity_file("assets/entities/data/enemy_nadino_2.dodjo", ENTITY_ENEMY_NADINO_2);
    // ---- Bosses ----
    add_entity_file("assets/entities/data/enemy_nadino_2.dodjo", ENTITY_ENEMY_BOSS);
    // ---- Chests ----
    add_entity_file("assets/entities/data/bronze_chest.dodjo", ENTITY_BRONZE_CHEST);
    add_entity_file("assets/entities/data/silver_chest.dodjo", ENTITY_SILVER_CHEST);
    add_entity_file("assets/entities/data/gold_chest.dodjo", ENTITY_GOLD_CHEST);
    add_entity_file("assets/entities/data/nadino_chest.dodjo", ENTITY_NADINO_CHEST);
    // ---- Others ----
    add_entity_file("assets/entities/data/star_gate.dodjo", ENTITY_STAR_GATE);
    // Add more entity files here...

    // Load chunk files
    // ---- Not Generated Chunks ----
    add_chunk_file("assets/chunks/debug.dodjo", CHUNK_DEBUG);
    add_chunk_file("assets/chunks/single.dodjo", CHUNK_SINGLE);
    add_chunk_file("assets/chunks/spawn.dodjo", CHUNK_SPAWN);
    // ---- Generated Chunks ----
    add_generated_chunk("assets/chunks/default.dodjo", CHUNK_DEFAULT);
    add_generated_chunk("assets/chunks/default2.dodjo", CHUNK_DEFAULT2);
    add_generated_chunk("assets/chunks/treasure_room_1.dodjo", CHUNK_TREASURE_ROOM);
    add_chunk_file("assets/chunks/boss_room.dodjo", CHUNK_BOSS_ROOM);
    add_generated_chunk("assets/chunks/waiting_room.dodjo", CHUNK_WAITING_ROOM);
    add_generated_chunk("assets/chunks/random_chunk_easy.dodjo", CHUNK_RANDOM_EASY);
    add_generated_chunk("assets/chunks/random_chunk_medium.dodjo", CHUNK_RANDOM_MEDIUM);
    add_generated_chunk("assets/chunks/random_chunk_hard.dodjo", CHUNK_RANDOM_HARD);
    add_generated_chunk("assets/chunks/random_chunk_nadinhard.dodjo", CHUNK_RANDOM_NADINHARD);
    add_chunk_file("assets/chunks/escape_room_1.dodjo", CHUNK_ESCAPE_ROOM_1);
    add_chunk_file("assets/chunks/escape_room_2.dodjo", CHUNK_ESCAPE_ROOM_2);
    // Add more chunk files here...

    // Load usable item files (First spec=rarity)
    // ---- Bows ----
    add_usable_item_file("assets/items/data/basic_bow.dodjo", USABLE_ITEM_BASIC_BOW);
    add_usable_item_file("assets/items/data/advanced_bow.dodjo", USABLE_ITEM_ADVANCED_BOW);
    add_usable_item_file("assets/items/data/super_bow.dodjo", USABLE_ITEM_SUPER_BOW);
    add_usable_item_file("assets/items/data/nadino_bow.dodjo", USABLE_ITEM_NADINO_BOW);
    // ---- Keys ----
    add_usable_item_file("assets/items/data/bronze_key.dodjo", USABLE_ITEM_BRONZE_KEY);
    add_usable_item_file("assets/items/data/silver_key.dodjo", USABLE_ITEM_SILVER_KEY);
    add_usable_item_file("assets/items/data/gold_key.dodjo", USABLE_ITEM_GOLD_KEY);
    add_usable_item_file("assets/items/data/nadino_key.dodjo", USABLE_ITEM_NADINO_KEY);
    // ---- Consumables ----
    add_usable_item_file("assets/items/data/onion_ring.dodjo", USABLE_ITEM_ONION_RING);
    add_usable_item_file("assets/items/data/stockfish.dodjo", USABLE_ITEM_STOCKFISH);
    add_usable_item_file("assets/items/data/school_dishes.dodjo", USABLE_ITEM_SCHOOL_DISHES);
    add_usable_item_file("assets/items/data/golden_apple.dodjo", USABLE_ITEM_GOLDEN_APPLE);
    add_usable_item_file("assets/items/data/forgotten_dish.dodjo", USABLE_ITEM_FORGOTTEN_DISH);
    // ---- Others ----
    add_usable_item_file("assets/items/data/bomb.dodjo", USABLE_ITEM_BOMB);
    // Add more usable item files here...
}