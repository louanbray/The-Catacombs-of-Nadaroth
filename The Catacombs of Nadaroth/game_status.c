#include "game_status.h"

#include "logger.h"

pthread_mutex_t pause_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pause_cond = PTHREAD_COND_INITIALIZER;
int GAME_PAUSED = 0;
static int DEBUG_MODE = 0;
static int RESET_NEEDED = 0;

void pause_game(void) {
    pthread_mutex_lock(&pause_mutex);
    GAME_PAUSED = 1;
    RESET_NEEDED = 1;
    LOG_INFO("Game paused");
    pthread_mutex_unlock(&pause_mutex);
}

void resume_game(void) {
    pthread_mutex_lock(&pause_mutex);
    GAME_PAUSED = 0;
    LOG_INFO("Game resumed");
    pthread_mutex_unlock(&pause_mutex);
    pthread_cond_signal(&pause_cond);
}

void set_debug_mode(int mode) {
    DEBUG_MODE = mode;
    LOG_INFO("Debug mode set to: %d", DEBUG_MODE);
}

int is_debug_mode(void) {
    return DEBUG_MODE;
}

int need_reset(void) {
    if (RESET_NEEDED) {
        RESET_NEEDED = 0;
        return 1;
    }
    return 0;
}