#include "player_handler.h"

#include <stdio.h>

#include "../game_objects/chunk.h"
#include "../game_objects/entity.h"
#include "../managers/achievements_manager.h"
#include "../managers/audio_manager.h"
#include "../managers/loot_manager.h"
#include "../managers/statistics_manager.h"

/// @brief Returns the part of the map the player is in as a direction (USED ONLY FOR GATES)
/// @param x player x
/// @param y player y
/// @return Direction direction
Direction get_direction(int x, int y) {
    if (x > 10) {
        return DIR_EAST;
    } else if (x < -10) {
        return DIR_WEST;
    } else if (y > 10) {
        return DIR_NORTH;
    } else if (y < -10) {
        return DIR_SOUTH;
    }
    return 0;
}

bool is_player_in_interaction_range(player* p, int x, int y) {
    return distance_to_player_sq(p, x, y) <= 13;
}

bool can_player_see(player* p, int target_x, int target_y, item* e_brain) {
    hm* chunk_items = get_chunk_furniture_coords(get_player_chunk(p));
    if (chunk_items == NULL) return false;

    int px = get_player_x(p);
    int py = get_player_y(p);

    if (px == target_x && py == target_y) return true;

    int dx = abs(target_x - px) / 2;
    int dy = abs(target_y - py);
    int sx = (px < target_x) ? 2 : -2;
    int sy = (py < target_y) ? 1 : -1;
    int err = dx - dy;

    int curr_x = px;
    int curr_y = py;

    while (true) {
        if (curr_x == target_x && curr_y == target_y) break;

        if (curr_x != px || curr_y != py) {
            item* obstacle = (item*)get_hm(chunk_items, curr_x, curr_y);
            if (obstacle != NULL) {
                //? if (is_item_solid(obstacle)) return false;
                if (!is_an_entity(obstacle)) return false;
                if (get_entity_brain(get_entity_link(obstacle)) != e_brain) return false;
            }
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            curr_x += sx;
        }
        if (e2 < dx) {
            err += dx;
            curr_y += sy;
        }
    }

    return true;
}

bool can_interact(player* p, item* e_brain) {
    int x = get_item_x(e_brain);
    int y = get_item_y(e_brain);
    int dx = x + RECENTER_X;
    int dy = RECENTER_Y - y;
    return is_player_in_interaction_range(p, dx, dy) && can_player_see(p, x, y, e_brain);
}

void open_lootable_from_chunk(chunk* c, item* i) {
    lootable* loot = get_item_spec(i);
    item* drop = generate_loot(loot);
    if (drop) {
        set_item_x(drop, get_item_x(i));
        set_item_y(drop, get_item_y(i));
        add_item(c, drop);
    }

    increment_statistic(STAT_CHEST_OPENED, 1);
    set_achievement_progress(ACH_TREASURE_SEEKER, 1);
    set_achievement_progress(ACH_LOOT_COLLECTOR, get_statistic(STAT_CHEST_OPENED));
    play_sound_effect_by_id(AUDIO_PICKUP_ITEM);
}

bool check_lootable_interaction(player* p, int x, int y) {
    item* i = (item*)get_hm(get_chunk_furniture_coords(get_player_chunk(p)), x - RECENTER_X, RECENTER_Y - y);
    if (!i) return false;
    if (get_item_type(i) != ITEMTYPE_LOOTABLE || !is_an_entity(i)) return false;
    entity* e = get_entity_link(i);
    item* it = get_entity_brain(e);
    lootable* loot = get_item_spec(it);

    hotbar* h = get_player_hotbar(p);

    int hotbar_index = get_hotbar_index_of_usable_item(h, loot->key);

    bool has_key = loot->key != USABLE_ITEM_NONE;

    if (has_key && hotbar_index == -1) return false;
    if (!can_interact(p, it)) return false;
    remove_entity_from_chunk(e);

    open_lootable_from_chunk(get_player_chunk(p), it);

    if (has_key) hotbar_drop(h, hotbar_index, true);
    destroy_entity(e);
    return true;
}

PlayerMovementResult collide_lootable(player* p, item* it) {
    hotbar* h = get_player_hotbar(p);
    item* key = get_selected_item(h);
    if (is_hotbar_full(h)) return MOV_CANT_MOVE;
    if (!is_an_entity(it)) {
        remove_item(get_player_chunk(p), it);
        return MOV_CAN_MOVE;
    }
    entity* e = get_entity_link(it);
    item* i = get_entity_brain(e);
    lootable* loot = get_item_spec(i);

    bool has_key = loot->key != USABLE_ITEM_NONE;
    if (has_key && !key) return MOV_CANT_MOVE;
    if (has_key && get_item_usable_type(key) != loot->key) return MOV_CANT_MOVE;
    remove_entity_from_chunk(e);

    open_lootable_from_chunk(get_player_chunk(p), i);

    if (has_key) hotbar_drop(h, get_selected_slot(h), true);
    destroy_entity(e);
    return MOV_PICKED_UP_ENTITY;
}

PlayerMovementResult pickup_from_chunk(player* p, item* i) {
    if (is_hotbar_full(get_player_hotbar(p))) return MOV_CANT_MOVE;

    bool isAnEntity = is_an_entity(i);

    if (isAnEntity) {
        entity* e = get_entity_link(i);

        i = get_entity_brain(e);

        remove_entity_from_chunk(e);
    } else {
        remove_item(get_player_chunk(p), i);
    }

    pickup(get_player_hotbar(p), i);

    return MOV_PICKED_UP + isAnEntity;
}

PlayerMovementResult handle(player* p, int x, int y) {
    chunk* ck = get_player_chunk(p);
    hm* h = get_chunk_furniture_coords(ck);

    item* i = get_hm(h, x, y);

    if (i != NULL) {
        if (is_item_hidden(i)) return MOV_CAN_MOVE;

        switch (get_item_type(i)) {  //? Modify to an item-player interaction
            case ITEMTYPE_GATE:
                move_player_chunk(p, get_direction(x, y));
                return MOV_MOVED_CHUNK;
                break;
            case ITEMTYPE_SGATE:
                move_player_chunk(p, DIR_STARGATE);
                return MOV_MOVED_CHUNK;
                break;
            case ITEMTYPE_LOOTABLE:
                return collide_lootable(p, i);
                break;
            case ITEMTYPE_PICKABLE:
                return pickup_from_chunk(p, i);
                break;
            default:
                return MOV_CANT_MOVE;
                break;
        }
    }

    return MOV_CAN_MOVE;
}