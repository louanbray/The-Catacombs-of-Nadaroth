#include "constants.h"

const char* CHUNK_NAMES[CHUNK_TYPE_COUNT] = {
    "DEBUG",
    "SINGLE",
    "SPAWN",
    "DEFAULT",
    "DEFAULT2",
    "TREASURE_ROOM",
    "BOSS_ROOM",
    "WAITING_ROOM",
    "RANDOM_EASY",
    "RANDOM_MEDIUM",
    "RANDOM_HARD",
    "RANDOM_NADINHARD",
    "ESCAPE_ROOM_1",
    "ESCAPE_ROOM_2",
};

const int ScorePerPhase[GAMEPHASE_COUNT] = {0, 25, 75, 210, 630, 0, 0};