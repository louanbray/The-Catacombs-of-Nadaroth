#ifndef CONSTANTS_H
#define CONSTANTS_H

/// @brief KEY_CODE Constants
#define KEY_Z_LOW 122
#define KEY_Z_HIGH 90
#define KEY_Q_LOW 113
#define KEY_Q_HIGH 81
#define KEY_S_LOW 115
#define KEY_S_HIGH 83
#define KEY_D_LOW 100
#define KEY_D_HIGH 68
#define KEY_W_LOW 119
#define KEY_W_HIGH 87
#define KEY_1 49
#define KEY_2 50
#define KEY_3 51
#define KEY_4 52
#define KEY_5 53
#define KEY_6 54
#define KEY_7 55
#define KEY_8 56
#define KEY_9 57
#define KEY_ARROW_UP 65
#define KEY_ARROW_DOWN 66
#define KEY_ARROW_RIGHT 67
#define KEY_ARROW_LEFT 68

/// @brief Render Constants
#define RENDER_WIDTH 129
#define RENDER_HEIGHT 41

/// @brief Utils
#define CHAR_TO_INT 49

//! COPYRIGHT 24/01/2025 20:33, EVERY IDEA HERE IS MINE IF YOU FOUND ANY LEAKED DATA PLEASE INFORM THE DEVELOPPER OF THE GAME

/*
TODO LIST:
- Make any key event stored in an input manager object so that you can check at any moment if a key is pressed                    [DEV] (HIGH)          {DONE}
- Update the screen render to make a part of the screen reserved to items descriptions and naration                               [RENDER] (HIGH)       {DONE}
- Enemy->Player Interaction                                                                                                       [DEV] (HIGH)          {DONE}
- Enemy PATHFINDING huh ? //! CHANGING GAME DESIGN, DO NOT IMPLEMENT                                                              [DEV] (HIGH->NONE)    {DONE}
- Implement the mecanics below / missing items                                                                                    [DEV] (MEDIUM)        {DONE}
- Work on the designs of the items and chunks                                                                                     [RENDER] (MEDIUM)     {DONE}
- Implement status menu                                                                                                           [RENDER/DEV] (LOW)
- Work on menus (!!Username!!, character selection, stat attribution aso...)                                                      [RENDER/DEV] (LOW)
- Add cinematics                                                                                                                  [RENDER] (LOW)        {DONE - Lore}
- [Rework render concept to add mental health effect (limited vision)] -> //?Better lore                                          [RENDER/DEV] (LOW)    {DONE}
- Add stats and achievements objects                                                                                              [RENDER/DEV] (LOW)
- Saving Games, opening and sharing                                                                                               [DEV/RENDER] (LOW)
- Update Entity / Chunk editor                                                                                                    [DEV] (LOW)
- Easter Eggs (Konami Code in the character choosing menu)   									                                  [DEV] (LOW)
*/

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
    WALL,                // Isn't reactive to player contacts or interactions
    GATE,                // The four directions gate
    SGATE,               // Star gate (all the stargates in a chunk are linked)
    PICKABLE,            // Self explanatory
    ENEMY,               // A 'Thing' with HPs
    LOOTABLE,            // Will fetch a loot table
} ItemType;

typedef enum Rarity {
    BRONZE,
    SILVER,
    GOLD,
    NADINO,
} Rarity;

/// @brief Items meant to be used
typedef enum UsableItem {
    NOT_USABLE_ITEM,
    BASIC_BOW,
    ADVANCED_BOW,
    SUPER_BOW,
    NADINO_BOW,
    BRONZE_KEY,
    SILVER_KEY,
    GOLD_KEY,
    NADINO_KEY,
    ONION_RING,      // Restore one heart (BRONZE)
    STOCKFISH,       // Restore full health (SILVER)
    SCHOOL_DISHES,   // Restore one mental health (SILVER)
    GOLDEN_APPLE,    // Add one heart (permanent (as long as you don't die you can regen)) (GOLD)
    BOMB,            // Wanna explode a chunk ? Here's what you need (BEWARE: NO LOOT WILL BE GIVEN | A NEW CHUNK WILL BE GENERATED HERE NEXT TIME (or the same if unlucky hehe)) (SILVER)
    FORGOTTEN_DISH,  // Restore full mental health (GOLD)
    //? Add armors / potions (consumables = stats up) if enough time

    USABLE_ITEM_COUNT
} UsableItem;

/// @brief chunk type (0,0) -> SPAWN
typedef enum ChunkType {  //? MODIFY TO ADD LEVELS
    SPAWN,
    DEFAULT,
    DEFAULT2,
    TREASURE_ROOM,           // Some chests but you need to hunt for keys (Chest value random | chest number and concept need to be worked on)  //! Keys are not implemented
    BOSS_ROOM,               // Depending on the boss, you need to find the right weapon | DIFFICULTY + = BETTER KEY REWARD (GOLD to NADINO)    //! WIP (Ig Boss == NADINO ?)
    WAITING_ROOM,            // Peaceful room                                                                                                   // I can manage this
    RANDOM_CHUNK_EASY,       // Randomly generated chunk (w enemies and bronze chests (1 max) | EASY)                                           //! WIP (NOT SO RANDOM FOR NOW)
    RANDOM_CHUNK_MEDIUM,     // Randomly generated chunk (w enemies and bronze chests (2 max) | MEDIUM)                                         //! WIP (NOT SO RANDOM FOR NOW)
    RANDOM_CHUNK_HARD,       // Randomly generated chunk (w enemies and bronze chests (3 max), 1 silver | HARD)                                 //! WIP (NOT SO RANDOM FOR NOW)
    RANDOM_CHUNK_NADINHARD,  // Randomly generated chunk (w enemies and silver chests (3 max) | VERY HARD)                                      //! WIP (NOT SO RANDOM FOR NOW)
    ESCAPE_ROOM_1,           // Escape rooms w enigmas, has a timer, do your best                                                               //! WIP (Timer ? What is that.)
    ESCAPE_ROOM_2,           // Same here, another variant                                                                                      //! WIP (Same here...)
    //? ADD AUTO GENERATED ESCAPE ROOMS (gimme the determination)

    CHUNK_TYPE_COUNT  // This will automatically be the count of enum entries
} ChunkType;

/// @brief Every entity (>1 item linked)
typedef enum EntityType {  //? MODIFY TO ADD ENTITY TYPES
    NULL_ENTITY,
    ENEMY_BRONZE_1,
    ENEMY_BRONZE_2,
    ENEMY_SILVER_1,
    ENEMY_SILVER_2,
    ENEMY_GOLD_1,
    ENEMY_GOLD_2,
    ENEMY_NADINO_1,
    ENEMY_NADINO_2,
    BRONZE_CHEST,
    SILVER_CHEST,
    GOLD_CHEST,
    NADINO_CHEST,
    STAR_GATE,

    // Other entity types...
    ENTITY_TYPE_COUNT  // This will automatically be the count of enum entries
} EntityType;

typedef enum GamePhase {
    INTRODUCTION,
    FIRST_ACT_FIRST_PHASE,
    FIRST_ACT_SECOND_PHASE,
    FIRST_ACT_THIRD_PHASE,
    FIRST_ACT_FOURTH_PHASE,
    FIRST_ACT_FIFTH_PHASE,
    FIRST_ACT_END
} GamePhase;

static const int ScorePerPhase[] = {0, 25, 75, 210, 630, 0, 0};

#endif