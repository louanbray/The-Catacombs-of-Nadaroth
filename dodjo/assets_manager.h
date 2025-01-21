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

/**
 * @struct AssetManager
 * @brief Manages the assets for different entity types.
 *
 * @var AssetManager::entities
 * Array of entity files indexed by EntityType.
 */
typedef struct AssetManager {
    EntityAssetFile* entities[ENTITY_TYPE_COUNT];  // Array of entity files indexed by EntityType
} AssetManager;

/**
 * Parses an entity file and returns an EntityAssetFile structure.
 *
 * @param filename The path to the entity file to parse.
 * @return A pointer to an EntityAssetFile structure containing the parsed data,
 *         or NULL if the file could not be parsed or memory allocation failed.
 */
EntityAssetFile* parse_entity_file(const char* filename);

/**
 * Creates and initializes a new AssetManager.
 *
 * @return A pointer to the newly created AssetManager, or NULL if memory allocation failed.
 */
AssetManager* create_asset_manager();

/**
 * Adds an entity file to the asset manager.
 *
 * @param manager A pointer to the AssetManager.
 * @param filename The path to the entity file to add.
 * @param type The type of the entity to add.
 * @return true if the entity file was successfully added, false otherwise.
 */
bool add_entity_file(AssetManager* manager, const char* filename, EntityType type);

/**
 * Retrieves an entity file from the asset manager.
 *
 * @param manager A pointer to the AssetManager.
 * @param type The type of the entity to retrieve.
 * @return A pointer to the EntityAssetFile corresponding to the specified type,
 *         or NULL if the type is invalid or the entity file is not found.
 */
EntityAssetFile* get_entity_file(AssetManager* manager, EntityType type);

/**
 * Frees all resources used by the asset manager.
 *
 * @param manager A pointer to the AssetManager to destroy.
 */
void destroy_asset_manager(AssetManager* manager);

#endif