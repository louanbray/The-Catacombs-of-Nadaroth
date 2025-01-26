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
 * @struct ChunkItem
 * @brief Represents an item within a chunk.
 *
 * @var ChunkItem::x
 * The x-coordinate of the item.
 *
 * @var ChunkItem::y
 * The y-coordinate of the item.
 *
 * @var ChunkItem::type
 * The type of the item.
 *
 * @var ChunkItem::display
 * The display status of the item.
 *
 * @var ChunkItem::row_repeat
 * The number of times the item is repeated in a row.
 *
 * @var ChunkItem::size
 * The size of the item.
 *
 * @var ChunkItem::col_repeat
 * The number of times the item is repeated in a column.
 *
 * @var ChunkItem::entity_type
 * The type of entity the item represents. (0 = NULL_ENTITY)
 *
 * @var ChunkItem::usable_item
 * The type of pickable item it is. (0 = NOT_USABLE_ITEM)
 */
typedef struct ChunkItem {
    int x, y, type, display, row_repeat, size, col_repeat, entity_type, usable_item;
} ChunkItem;

/**
 * @struct ChunkAssetFile
 * @brief Represents a file containing chunk assets.
 *
 * @var ChunkAssetFile::items
 * Pointer to an array of ChunkItem structures.
 *
 * @var ChunkAssetFile::item_count
 * The number of items in the array.
 */
typedef struct ChunkAssetFile {
    ChunkItem* items;
    size_t item_count;
} ChunkAssetFile;

/**
 * @struct UsableItemSpecs
 * @brief Represents the specifications of a usable item.
 *
 * This structure holds a dynamic array of specifications and the count of these specifications.
 *
 * @var UsableItemSpecs::specs
 * Dynamic array of specifications.
 *
 * @var UsableItemSpecs::spec_count
 * Number of specifications in the dynamic array.
 */
typedef struct UsableItemSpecs {
    int* specs;         // Dynamic array of specs
    size_t spec_count;  // Number of specs
} UsableItemSpecs;

/**
 * @struct UsableItemAssetFile
 * @brief Represents a usable item asset file.
 *
 * This structure contains the specifications, title, and description of a usable item.
 *
 * @var UsableItemAssetFile::specs
 * Specifications of the usable item.
 *
 * @var UsableItemAssetFile::title
 * Title of the usable item.
 *
 * @var UsableItemAssetFile::description
 * Description of the usable item.
 */
typedef struct UsableItemAssetFile {
    UsableItemSpecs specs;
    char* title;
    char* description;
} UsableItemAssetFile;

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
    UsableItemAssetFile* usable_items[USABLE_ITEM_COUNT];
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
 * @brief Adds a usable item file to the assets manager.
 *
 * This function takes the filename of a usable item and its type, and adds it to the assets manager.
 *
 * @param filename The name of the file to be added.
 * @param type The type of the usable item.
 * @return true if the file was successfully added, false otherwise.
 */
bool add_usable_item_file(const char* filename, UsableItem type);

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
 * @brief Retrieves the file associated with a given usable item type.
 *
 * This function takes a UsableItem type as input and returns a pointer to the
 * corresponding UsableItemAssetFile.
 *
 * @param type The type of the usable item for which the file is to be retrieved.
 * @return A pointer to the UsableItemAssetFile corresponding to the given type.
 */
UsableItemAssetFile* get_usable_item_file(UsableItem type);

/**
 * Frees all resources used by the asset manager.
 */
void destroy_asset_manager();

#endif