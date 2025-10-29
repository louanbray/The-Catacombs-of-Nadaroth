#include "game_status.h"

#include "logger.h"

pthread_mutex_t pause_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pause_cond = PTHREAD_COND_INITIALIZER;
int GAME_PAUSED = 0;
static int DEBUG_MODE = 0;
static int RESET_NEEDED = 0;
static struct timeval GAME_STARTED = {0, 0};  // Could be useful who knows (currently unused)
static struct timeval TIME_PLAYED = {0, 0};

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

void set_game_started(struct timeval started) {
    GAME_STARTED = started;
    LOG_INFO("Game started set to: %u seconds and %u microseconds", (unsigned int)started.tv_sec, (unsigned int)started.tv_usec);
}

void add_time_played(struct timeval delta) {
    TIME_PLAYED.tv_sec += delta.tv_sec;
    TIME_PLAYED.tv_usec += delta.tv_usec;
    if (TIME_PLAYED.tv_usec >= 1000000) {
        TIME_PLAYED.tv_sec += TIME_PLAYED.tv_usec / 1000000;
        TIME_PLAYED.tv_usec = TIME_PLAYED.tv_usec % 1000000;
    }
}

void set_time_played(struct timeval time_played) {
    TIME_PLAYED = time_played;
    LOG_INFO("Time played set to: %u seconds and %u microseconds", (unsigned int)time_played.tv_sec, (unsigned int)time_played.tv_usec);
}

struct timeval get_game_started(void) {
    return GAME_STARTED;
}

struct timeval get_time_played(void) {
    return TIME_PLAYED;
}