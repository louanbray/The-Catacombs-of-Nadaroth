#ifndef BEHAVIOUR_MANAGER_H
#define BEHAVIOUR_MANAGER_H

#include <stdbool.h>

#include "../utils/constants.h"

#define ATTACK_COOLDOWN_DEFAULT_RANDOM -1
#define ATTACK_COOLDOWN_DEFAULT_NOT_RANDOM -2

typedef struct Render_Buffer Render_Buffer;
typedef struct player player;
typedef struct item item;

typedef void (*attack_fn)(Render_Buffer* render_buffer, player* player, item* brain, int* attack_cooldown);

attack_fn get_attack_fn(EntityType etype);

void set_enemy_random_cooldown_enabled(bool state);
bool is_enemy_random_cooldown_enabled();

#endif  // BEHAVIOUR_MANAGER_H