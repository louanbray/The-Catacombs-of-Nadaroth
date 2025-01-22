#ifndef ASSETS_MANAGER_H
#define ASSETS_MANAGER_H

#include <stdbool.h>
#include <stdlib.h>

#include "constants.h"

/**
 * @struct EntitySpecs
 * @brief Represents the specifications of an entity.
 *
 * @var EntitySpecs::specs
 * Dynamic array of specifications.
 *
 * @var EntitySpecs::spec_count
 * Number of specifications.
 */
typedef struct EntitySpecs {
    int* specs;         // Dynamic array of specs
    size_t spec_count;  // Number of specs
} EntitySpecs;

/**
 * @struct EntityPart
 * @brief Represents a part of an entity.
 *
 * @var EntityPart::x
 * X-coordinate of the entity part.
 *
 * @var EntityPart::y
 * Y-coordinate of the entity part.
 *
 * @var EntityPart::display
 * Display flag for the entity part.
 *
 * @var EntityPart::row_repeat
 * Number of times the part is repeated in rows.
 *
 * @var EntityPart::col_repeat
 * Number of times the part is repeated in columns.
 */
typedef struct EntityPart {
    int x;
    int y;
    int display;
    int row_repeat;
    int col_repeat;
} EntityPart;

/**
 * @struct EntityAssetFile
 * @brief Represents an asset file for an entity.
 *
 * @var EntityAssetFile::specs
 * Entity properties from [SPECS].
 *
 * @var EntityAssetFile::count
 * Number of parts in the entity.
 *
 * @var EntityAssetFile::parts
 * Array of entity parts.
 */
typedef struct EntityAssetFile {
    EntitySpecs specs;  // Entity properties from [SPECS]
    int count;          // Number of parts
    EntityPart* parts;  // Array of entity parts
} EntityAssetFile;

typedef struct ChunkItem {
    int x, y, type, display, row_repeat, size, col_repeat, entity_type;
} ChunkItem;

typedef struct ChunkAssetFile {
    ChunkItem* items;
    size_t item_count;
} ChunkAssetFile;

/**
 * @struct AssetManager
 * @brief Manages the assets for different entity types.
 *
 * @var AssetManager::entities
 * Array of entity files indexed by EntityType.
 */
typedef struct AssetManager {
    EntityAssetFile* entities[ENTITY_TYPE_COUNT];  // Array of entity files indexed by EntityType
    ChunkAssetFile* chunks[CHUNK_TYPE_COUNT];
} AssetManager;

/**
 * @brief Initializes the asset management system.
 *
 * This function initializes the asset management system, preparing it for use.
 */
void init_assets_system();

/**
 * Adds an entity file to the asset manager.
 *
 * @param filename The path to the entity file to add.
 * @param type The type of the entity to add.
 * @return true if the entity file was successfully added, false otherwise.
 */
bool add_entity_file(const char* filename, EntityType type);

/**
 * @brief Adds a chunk file to the asset manager.
 *
 * This function adds a chunk file specified by the filename and type to the asset manager.
 *
 * @param filename The name of the file to be added.
 * @param type The type of the chunk file.
 * @return true if the file was successfully added, false otherwise.
 */
bool add_chunk_file(const char* filename, ChunkType type);

/**
 * Retrieves an entity file from the asset manager.
 *
 * @param type The type of the entity to retrieve.
 * @return A pointer to the EntityAssetFile corresponding to the specified type,
 *         or NULL if the type is invalid or the entity file is not found.
 */
EntityAssetFile* get_entity_file(EntityType type);

/**
 * @brief Retrieves a chunk file from the asset manager.
 *
 * This function retrieves a chunk file of the specified type from the asset manager.
 *
 * @param type The type of the chunk file to retrieve.
 * @return A pointer to the ChunkAssetFile if found, NULL otherwise.
 */
ChunkAssetFile* get_chunk_file(ChunkType type);

/**
 * Frees all resources used by the asset manager.
 */
void destroy_asset_manager();

#endif