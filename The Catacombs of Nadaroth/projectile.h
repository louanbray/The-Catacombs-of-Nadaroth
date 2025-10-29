#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <stdbool.h>

typedef struct Render_Buffer Render_Buffer;
typedef struct player player;
typedef struct item item;

void fire_projectile(Render_Buffer* screen, player* p, int target_x, int target_y);

// Initialize the projectile system
void init_projectile_system(Render_Buffer* screen, player* p, int seed);

void kill_all_projectiles(Render_Buffer* screen);

void bow_check_flag();

void simulate_projectile_hit(int damage, player* p, Render_Buffer* screen);

void add_total_enemies(player* p);

#endif