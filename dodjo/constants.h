#ifndef CONSTANTS_H
#define CONSTANTS_H

/// @brief Gate position/type
enum Direction {  //! DO NOT MODIFY
    STARGATE,
    EAST,
    NORTH,
    WEST,
    SOUTH
};

/// @brief item type
enum ItemType {  //? MODIFY to add different types of items
    WALL,
    GATE,
    SGATE,
    PICKABLE,
    ENEMY,
};

/// @brief chunk type (0,0) -> SPAWN
enum ChunkType {  //? MODIFY TO ADD LEVELS
    SPAWN,
    DEFAULT,
    DEFAULT2,
    LABY,
};

#endif