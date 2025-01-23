#ifndef CONSTANTS_H
#define CONSTANTS_H

/// @brief Render Constants
#define RENDER_WIDTH 129
#define RENDER_HEIGHT 40

/// @brief Gate position/type
typedef enum Direction {  //! DO NOT MODIFY
    STARGATE,
    EAST,
    NORTH,
    WEST,
    SOUTH
} Direction;

/// @brief item type
typedef enum ItemType {  //? MODIFY TO ADD ITEM TYPES
    WALL,
    GATE,
    SGATE,
    PICKABLE,
    ENEMY,
} ItemType;

/// @brief chunk type (0,0) -> SPAWN
typedef enum ChunkType {  //? MODIFY TO ADD LEVELS
    SPAWN,
    DEFAULT,
    DEFAULT2,
    LABY,
    CHUNK_TYPE_COUNT  // This will automatically be the count of enum entries
} ChunkType;

typedef enum EntityType {  //? MODIFY TO ADD ENTITY TYPES
    NULL_ENTITY,
    ENEMY1,
    ENEMY2,
    // Other entity types...
    ENTITY_TYPE_COUNT  // This will automatically be the count of enum entries
} EntityType;

#endif