#ifndef CUTSCENE_MANAGER_H
#define CUTSCENE_MANAGER_H

#include <stdbool.h>

typedef enum CutsceneID {
    CUTSCENE_NONE,
    CUTSCENE_TEST,

    CUTSCENE_COUNT,
} CutsceneID;

typedef struct Render_Buffer Render_Buffer;
typedef struct player player;

void start_cutscene(CutsceneID id, Render_Buffer* screen, player* original_player);
void init_cutscenes();
bool is_in_cutscene();
void update_cutscenes(Render_Buffer* screen, player* main_player);
void request_cutscene(CutsceneID id);
int get_cutscene_speed();

#endif  // CUTSCENE_MANAGER_H