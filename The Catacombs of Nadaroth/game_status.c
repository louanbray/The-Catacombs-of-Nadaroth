#include "game_status.h"

pthread_mutex_t pause_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pause_cond = PTHREAD_COND_INITIALIZER;
int GAME_PAUSED = 0;

void pause_game(void) {
    pthread_mutex_lock(&pause_mutex);
    GAME_PAUSED = 1;
    pthread_mutex_unlock(&pause_mutex);
}

void resume_game(void) {
    pthread_mutex_lock(&pause_mutex);
    GAME_PAUSED = 0;
    pthread_mutex_unlock(&pause_mutex);
    pthread_cond_signal(&pause_cond);
}
