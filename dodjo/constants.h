#ifndef CONSTANTS_H
#define CONSTANTS_H

/// @brief Gate position/type
typedef enum Direction {  //! DO NOT MODIFY
    STARGATE,
    EAST,
    NORTH,
    WEST,
    SOUTH
} Direction;

/// @brief item type
typedef enum ItemType {  //? MODIFY to add different types of items
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
} ChunkType;

typedef enum {
    ENEMY1,
    ENEMY2,
    // Other entity types...
    ENTITY_TYPE_COUNT  // This will automatically be the count of enum entries
} EntityType;

/// @brief Render Constants
#define RENDER_WIDTH 129
#define RENDER_HEIGHT 40

#endif