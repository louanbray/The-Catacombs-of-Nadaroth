#ifndef GAME_STATUS_H
#define GAME_STATUS_H

#include <pthread.h>

// Global synchronization variables
extern pthread_mutex_t pause_mutex;
extern pthread_cond_t pause_cond;
extern int GAME_PAUSED;

void pause_game(void);
void resume_game(void);

#endif