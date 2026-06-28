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

#define RECENTER_X (RENDER_WIDTH / 2 + 1)
#define RECENTER_Y (RENDER_HEIGHT / 2 - 1)

#define PLAYBOX_MIN_OX -64
#define PLAYBOX_MAX_OX 63
#define PLAYBOX_MIN_OY -17
#define PLAYBOX_MAX_OY 17

/// @brief Utils
#define CHAR_TO_INT 49

/// @brief Player designs
#define PLAYER_DESIGN_BALL 9210
#define PLAYER_DESIGN_CAMO 11201
#define PLAYER_DESIGN_BRAWLER 9632
#define PLAYER_DESIGN_SHIELD 9960

/// @brief Inventory
#define HOTBAR_SIZE 9

/// @brief Cinematics
#define CINEMATIC_FRAME_DELAY 1000000

#define MAX_MENTAL_HEALTH 4

//! COPYRIGHT 24/01/2025 20:33, EVERY IDEA HERE IS MINE IF YOU FOUND ANY LEAKED DATA PLEASE INFORM THE DEVELOPPER OF THE GAME

/*
TODO LIST:
- Add Perfectionist achievement mechanic (Enemies alive;-= when killed;!=+=when !is_chunk_generated())                            [DEV] (LOW)           {DONE}
//- Predetermined actions for seed and saving seed                                                                                [DEV] (LOW)
- Pause menu with options (Resume, Save, Load, Quit, Stats, Ach, Help)                                                            [RENDER/DEV] (MEDIUM) {DONE}
- Make any key event stored in an input manager object so that you can check at any moment if a key is pressed                    [DEV] (HIGH)          {DONE}
- Update the screen render to make a part of the screen reserved to items descriptions and narration                              [RENDER] (HIGH)       {DONE}
- Enemy->Player Interaction                                                                                                       [DEV] (HIGH)          {DONE}
- Enemy PATHFINDING huh ? //// CHANGING GAME DESIGN, DO NOT IMPLEMENT                                                             [DEV] (HIGH->NONE)    {DONE}
- Implement the mechanics below / missing items                                                                                   [DEV] (MEDIUM)        {DONE}
- Work on the designs of the items and chunks                                                                                     [RENDER] (MEDIUM)     {DONE}
- Add cinematics                                                                                                                  [RENDER] (LOW)        {DONE}
- [Rework render concept to add mental health effect (limited vision)] -> Better lore                                             [RENDER/DEV] (LOW)    {DONE}
- Add stats and achievements objects                                                                                              [DEV] (LOW)           {DONE}
- Have a way to render achievements and stats on the screen                                                                       [RENDER] (LOW)        {DONE}
- Saving Games, opening and sharing                                                                                               [DEV/RENDER] (LOW)    {DONE}
- Update Entity / Chunk editor                                                                                                    [DEV] (LOW)           {DONE}
- Change time_played timer precision to microseconds + fix timer reset when loading a save                                        [DEV] (MEDIUM)        {DONE}
- Separate STAT_TIME_PLAYED and time_in_game (pause doesn't count) -> can restore exact time elapsed                              [DEV] (LOW)           {DONE}
? - Implement status menu                                                                                                         [RENDER/DEV] (LOW)
? - Work on menus (!!Username!!, stat attribution aso...)                                                                         [RENDER/DEV] (LOW)
? - Easter Eggs (Konami Code in the character choosing menu)   									                                  [DEV] (LOW)
*/

/// @brief Gate position/type
typedef enum Direction {  //! DO NOT MODIFY
    DIR_STARGATE,
    DIR_EAST,
    DIR_NORTH,
    DIR_WEST,
    DIR_SOUTH
} Direction;

/// @brief item type
typedef enum ItemType {  //? MODIFY TO ADD ITEM TYPES
    ITEMTYPE_WALL,       // Isn't reactive to player contacts or interactions
    ITEMTYPE_GATE,       // The four directions gate
    ITEMTYPE_SGATE,      // Star gate (all the stargates in a chunk are linked)
    ITEMTYPE_PICKABLE,   // Self explanatory
    ITEMTYPE_ENEMY,      // A 'Thing' with HPs
    ITEMTYPE_LOOTABLE,   // Will fetch a loot table
} ItemType;

typedef enum Rarity {
    RARITY_BRONZE,
    RARITY_SILVER,
    RARITY_GOLD,
    RARITY_NADINO,
} Rarity;

/// @brief Items meant to be used
typedef enum UsableItem {
    USABLE_ITEM_NOT_USABLE,

    // Bows
    USABLE_ITEM_BASIC_BOW,
    USABLE_ITEM_ADVANCED_BOW,
    USABLE_ITEM_SUPER_BOW,
    USABLE_ITEM_NADINO_BOW,
    USABLE_ITEM_BOWS_END,  // Marker to separate bows from other usable items

    // Other
    USABLE_ITEM_BRONZE_KEY,
    USABLE_ITEM_SILVER_KEY,
    USABLE_ITEM_GOLD_KEY,
    USABLE_ITEM_NADINO_KEY,
    USABLE_ITEM_BOMB,  // Wanna explode a chunk ? Here's what you need (BEWARE: NO LOOT WILL BE GIVEN | A NEW CHUNK WILL BE GENERATED HERE NEXT TIME (or the same if unlucky hehe)) (SILVER)
    //? Add armors / potions (consumables = stats up) if enough time

    // Foods
    USABLE_ITEM_FOOD_START,      // Marker to separate food from other usable items
    USABLE_ITEM_ONION_RING,      // Restore one heart (BRONZE)
    USABLE_ITEM_STOCKFISH,       // Restore full health (SILVER)
    USABLE_ITEM_SCHOOL_DISHES,   // Restore one mental health (SILVER)
    USABLE_ITEM_GOLDEN_APPLE,    // Add one heart (permanent (as long as you don't die you can regen)) (GOLD)
    USABLE_ITEM_FORGOTTEN_DISH,  // Restore full mental health (GOLD)

    USABLE_ITEM_COUNT
} UsableItem;

/// @brief chunk type (0,0) -> SPAWN
typedef enum ChunkType {  //? MODIFY TO ADD LEVELS
    CHUNK_DEBUG,
    CHUNK_SPAWN,
    CHUNK_DEFAULT,
    CHUNK_DEFAULT2,
    CHUNK_TREASURE_ROOM,     // Some chests but you need to hunt for keys (Chest value random | chest number and concept need to be worked on)  //! Keys are not implemented
    CHUNK_BOSS_ROOM,         // Depending on the boss, you need to find the right weapon | DIFFICULTY + = BETTER KEY REWARD (GOLD to NADINO)    //! WIP (Ig Boss == NADINO ?)
    CHUNK_WAITING_ROOM,      // Peaceful room                                                                                                   // I can manage this
    CHUNK_RANDOM_EASY,       // Randomly generated chunk (w enemies and bronze chests (1 max) | EASY)                                           //! WIP (NOT SO RANDOM FOR NOW)
    CHUNK_RANDOM_MEDIUM,     // Randomly generated chunk (w enemies and bronze chests (2 max) | MEDIUM)                                         //! WIP (NOT SO RANDOM FOR NOW)
    CHUNK_RANDOM_HARD,       // Randomly generated chunk (w enemies and bronze chests (3 max), 1 silver | HARD)                                 //! WIP (NOT SO RANDOM FOR NOW)
    CHUNK_RANDOM_NADINHARD,  // Randomly generated chunk (w enemies and silver chests (3 max) | VERY HARD)                                      //! WIP (NOT SO RANDOM FOR NOW)
    CHUNK_ESCAPE_ROOM_1,     // Escape rooms w enigmas, has a timer, do your best                                                               //! WIP (Timer ? What is that.)
    CHUNK_ESCAPE_ROOM_2,     // Same here, another variant                                                                                      //! WIP (Same here...)
    //? ADD AUTO GENERATED ESCAPE ROOMS (gimme the determination)

    CHUNK_TYPE_COUNT  // This will automatically be the count of enum entries
} ChunkType;

/// @brief Every entity (>1 item linked)
typedef enum EntityType {  //? MODIFY TO ADD ENTITY TYPES
    ENTITY_NULL,
    ENTITY_ENEMY_BRONZE_1,
    ENTITY_ENEMY_BRONZE_2,
    ENTITY_ENEMY_SILVER_1,
    ENTITY_ENEMY_SILVER_2,
    ENTITY_ENEMY_GOLD_1,
    ENTITY_ENEMY_GOLD_2,
    ENTITY_ENEMY_NADINO_1,
    ENTITY_ENEMY_NADINO_2,
    ENTITY_BRONZE_CHEST,
    ENTITY_SILVER_CHEST,
    ENTITY_GOLD_CHEST,
    ENTITY_NADINO_CHEST,
    ENTITY_STAR_GATE,

    // Other entity types...
    ENTITY_TYPE_COUNT  // This will automatically be the count of enum entries
} EntityType;

typedef enum GamePhase {
    GAMEPHASE_INTRODUCTION,
    GAMEPHASE_FIRST_ACT_FIRST_PHASE,
    GAMEPHASE_FIRST_ACT_SECOND_PHASE,
    GAMEPHASE_FIRST_ACT_THIRD_PHASE,
    GAMEPHASE_FIRST_ACT_FOURTH_PHASE,
    GAMEPHASE_FIRST_ACT_END
} GamePhase;

// Color enumeration (using a small integer type)
typedef enum Color {
    COLOR_DEFAULT = 0,
    COLOR_MAGENTA_BOLD,
    COLOR_RED,
    COLOR_YELLOW,
    COLOR_CYAN_BOLD,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_MAGENTA,
    COLOR_GRAY,
    // Extend as needed
} Color;

// Game State after exiting the pause menu
typedef enum ResumeState {
    RESUME_DEFAULT,
    RESUME_NEW_GAME,
    RESUME_RESET
} ResumeState;

/// @brief Player Movement results
typedef enum PlayerMovementResult {
    MOV_CAN_MOVE,
    MOV_MOVED_CHUNK,
    MOV_CANT_MOVE,
    MOV_PICKED_UP,
    MOV_PICKED_UP_ENTITY
} PlayerMovementResult;

///@brief Player Class
typedef enum PlayerClass {
    PLAYER_CLASS_BALL,
    PLAYER_CLASS_CAMO,
    PLAYER_CLASS_BRAWLER,
    PLAYER_CLASS_SHIELD,
} PlayerClass;

static const int ScorePerPhase[] = {0, 25, 75, 210, 630, 0, 0};  //! TEMPORARY (1870 removed)

#endif