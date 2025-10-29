#ifndef GAME_STATUS_H
#define GAME_STATUS_H

#include <pthread.h>
#include <sys/time.h>

extern pthread_mutex_t pause_mutex;
extern pthread_cond_t pause_cond;
extern int GAME_PAUSED;

void pause_game(void);
void resume_game(void);
void set_debug_mode(int mode);
int is_debug_mode(void);
int need_reset(void);
void set_game_started(struct timeval started);
struct timeval get_game_started(void);
struct timeval get_time_played(void);
void add_time_played(struct timeval delta);
void set_time_played(struct timeval time_played);

#endif