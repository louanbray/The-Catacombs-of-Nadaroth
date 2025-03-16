#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <stdbool.h>

typedef struct Render_Buffer Render_Buffer;
typedef struct player player;
typedef struct item item;

void fire_projectile(Render_Buffer* r, player* p, int target_x, int target_y);

// Initialize the projectile system
void init_projectile_system(Render_Buffer* r, player* p);

#endif