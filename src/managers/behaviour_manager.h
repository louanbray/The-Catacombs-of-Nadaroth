#ifndef BEHAVIOUR_MANAGER_H
#define BEHAVIOUR_MANAGER_H

#include "../utils/constants.h"

typedef struct Render_Buffer Render_Buffer;
typedef struct player player;
typedef struct item item;

typedef void (*attack_fn)(Render_Buffer* render_buffer, player* player, item* brain, int* attack_cooldown);

attack_fn get_attack_fn(EntityType etype);

#endif  // BEHAVIOUR_MANAGER_H