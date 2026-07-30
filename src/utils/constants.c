#include "constants.h"

#define CHUNK_TABLE_ENTRY(name) {name, sizeof(name) / sizeof(*(name))}

const int ScorePerPhase[GAMEPHASE_COUNT] = {0, 25, 75, 210, 630, 0, 0};

const char* CHUNK_NAMES[CHUNK_TYPE_COUNT] = {
    "EMPTY",
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

static const ChunkType introduction_chunks[] = {
    CHUNK_WAITING_ROOM,
    CHUNK_DEFAULT,
    CHUNK_ESCAPE_ROOM_1,
};

static const ChunkType first_act_first_phase_chunks[] = {
    CHUNK_WAITING_ROOM,
    CHUNK_DEFAULT,
    CHUNK_DEFAULT2,
    CHUNK_ESCAPE_ROOM_1,
    CHUNK_ESCAPE_ROOM_2,
    CHUNK_RANDOM_EASY,
};

static const ChunkType first_act_second_phase_chunks[] = {
    CHUNK_WAITING_ROOM,
    CHUNK_DEFAULT,
    CHUNK_TREASURE_ROOM,
    CHUNK_ESCAPE_ROOM_1,
    CHUNK_ESCAPE_ROOM_2,
    CHUNK_RANDOM_EASY,
    CHUNK_RANDOM_MEDIUM,
};

static const ChunkType first_act_third_phase_chunks[] = {
    CHUNK_WAITING_ROOM,
    CHUNK_TREASURE_ROOM,
    CHUNK_RANDOM_EASY,
    CHUNK_RANDOM_MEDIUM,
    CHUNK_RANDOM_HARD,
};

static const ChunkType first_act_fourth_phase_chunks[] = {
    CHUNK_WAITING_ROOM,
    CHUNK_RANDOM_MEDIUM,
    CHUNK_RANDOM_HARD,
    CHUNK_RANDOM_NADINHARD,
};

static const ChunkType first_act_end_chunks[] = {
    CHUNK_SINGLE,
};

static const ChunkType wip_chunks[] = {
    CHUNK_DEFAULT,
    CHUNK_DEFAULT2,
};

const ChunkTable CHUNK_TABLE[GAMEPHASE_COUNT] = {
    [GAMEPHASE_INTRODUCTION] = CHUNK_TABLE_ENTRY(introduction_chunks),
    [GAMEPHASE_FIRST_ACT_FIRST_PHASE] = CHUNK_TABLE_ENTRY(first_act_first_phase_chunks),
    [GAMEPHASE_FIRST_ACT_SECOND_PHASE] = CHUNK_TABLE_ENTRY(first_act_second_phase_chunks),
    [GAMEPHASE_FIRST_ACT_THIRD_PHASE] = CHUNK_TABLE_ENTRY(first_act_third_phase_chunks),
    [GAMEPHASE_FIRST_ACT_FOURTH_PHASE] = CHUNK_TABLE_ENTRY(first_act_fourth_phase_chunks),
    [GAMEPHASE_FIRST_ACT_END] = CHUNK_TABLE_ENTRY(first_act_end_chunks),
    [GAMEPHASE_WIP] = CHUNK_TABLE_ENTRY(wip_chunks),
};