#include "behaviour_manager.h"

#include "../display/render.h"
#include "../game_objects/item.h"
#include "../game_objects/player.h"
#include "projectile_manager.h"

void send_projectile(Render_Buffer* r, player* p, int from_x, int from_y, int target_x, int target_y, int damage, int ignore_display, int speed, unsigned int design, int range, bool infinity, ProjectileCallback callback) {
    from_x = CLAMP(from_x, 1, RENDER_WIDTH);
    from_y = CLAMP(from_y, 1, PLAY_HEIGHT_MAX);

    projectile_data* callback_data = malloc(sizeof(projectile_data));
    if (!callback_data) return;
    callback_data->x0 = from_x;
    callback_data->y0 = from_y;
    callback_data->damage = damage;
    callback_data->p = p;
    callback_data->screen = r;

    if (!spawn_projectile(from_x, from_y, target_x, target_y, ignore_display, speed, design, range, infinity, callback, callback_data)) {
        free(callback_data);
    }
}

void send_projectile_with_enemy_base(Render_Buffer* r, player* p, item* enemy_brain, int from_x, int from_y, int target_x, int target_y) {
    enemy* e = (enemy*)get_item_spec(enemy_brain);

    send_projectile(r, p, from_x, from_y, target_x, target_y, e->damage, get_item_display(enemy_brain), e->speed, ENEMY_PROJECTILE_DESIGN, -1, e->infinity, enemy_attack_callback);
}

void send_projectile_from_enemy(Render_Buffer* r, player* p, item* enemy_brain, int target_x, int target_y) {
    int enemy_x = WTI_X(get_item_x(enemy_brain));
    int enemy_y = WTI_Y(get_item_y(enemy_brain));
    send_projectile_with_enemy_base(r, p, enemy_brain, enemy_x, enemy_y, target_x, target_y);
}

void send_projectile_from_enemy_relative(Render_Buffer* r, player* p, item* enemy_brain, int rel_x, int rel_y) {
    int target_x = WTI_X(get_item_x(enemy_brain)) + rel_x;
    int target_y = WTI_Y(get_item_y(enemy_brain)) + rel_y;
    send_projectile_from_enemy(r, p, enemy_brain, target_x, target_y);
}

void send_projectile_from_enemy_to_player(Render_Buffer* r, player* p, item* enemy_brain, bool check_aggro) {
    int player_x = WTI_X(get_player_x(p));
    int player_y = WTI_Y(get_player_y(p));
    if (check_aggro && !is_player_aggroed(p, WTI_X(get_item_x(enemy_brain)), WTI_Y(get_item_y(enemy_brain)))) return;
    send_projectile_from_enemy(r, p, enemy_brain, player_x, player_y);
}

void default_enemy_behaviour(Render_Buffer* r, player* p, item* enemy_brain, int* attack_cooldown) {
    if (!r || !p || !enemy_brain) return;
    *attack_cooldown = -1;
    send_projectile_from_enemy_to_player(r, p, enemy_brain, true);
}

void boss_behaviour(Render_Buffer* r, player* p, item* enemy_brain, int* attack_cooldown) {
    if (!r || !p || !enemy_brain) return;

    int pattern = RAND_RANGE(0, 2);

    switch (pattern) {
        case 0: {
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;

                    send_projectile_from_enemy_relative(r, p, enemy_brain, dx * 60, dy * 30);
                }
            }
            *attack_cooldown = 90 / 4;
            break;
        }

        case 1: {
            int player_x = WTI_X(get_player_x(p));
            int player_y = WTI_Y(get_player_y(p));

            send_projectile_from_enemy(r, p, enemy_brain, player_x, player_y);
            send_projectile_from_enemy(r, p, enemy_brain, player_x - 4, player_y);
            send_projectile_from_enemy(r, p, enemy_brain, player_x + 4, player_y);

            *attack_cooldown = 45 / 4;
            break;
        }

        case 2: {
            for (int i = 0; i < 8; i++) {
                int spawn_x = RAND_RANGE(2, RENDER_WIDTH - 1);
                send_projectile_with_enemy_base(r, p, enemy_brain, spawn_x, 2, spawn_x, PLAY_HEIGHT_MAX);
            }
            *attack_cooldown = 30 / 4;
            break;
        }

        default:
            send_projectile_from_enemy_to_player(r, p, enemy_brain, false);
            *attack_cooldown = 60 / 4;
            break;
    }
}

attack_fn get_attack_fn(EntityType etype) {
    switch (etype) {
        case ENTITY_ENEMY_BOSS:
            return boss_behaviour;
        default:
            return default_enemy_behaviour;
    }
}