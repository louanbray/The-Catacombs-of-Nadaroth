#include "handler.h"

#include <stdio.h>

#include "generation.h"

/// @brief Return the part of the map the player is in as a direction
/// @param x player x
/// @param y player y
/// @return int direction
int get_direction(int x, int y) {
    if (x > 10) {
        return EAST;
    }
    if (x < -10) {
        return WEST;
    }
    if (y > 10) {
        return NORTH;
    }
    if (y < -10) {
        return SOUTH;
    }
    return 0;
}

int pickup_from_chunk(player* p, item* i) {
    if (is_hotbar_full(get_player_hotbar(p))) return 2;
    pickup(get_player_hotbar(p), i);
    remove_item(get_player_chunk(p), i);
    return 3;
}

int handle(player* p, int x, int y) {
    chunk* ck = get_player_chunk(p);
    hm* h = get_chunk_furniture_coords(ck);
    item* i = get_hm(h, x, y);
    if (i != NULL) {
        if (is_item_hidden(i)) return 0;
        switch (get_item_type(i)) {
            case GATE:
                move_player_chunk(p, get_direction(x, y));
                return 1;
                break;
            case SGATE:
                move_player_chunk(p, STARGATE);
                return 1;
                break;
            case PICKABLE:
                return pickup_from_chunk(p, i);
                break;
            default:
                return 2;
                break;
        }
    }
    return 0;
}

bool is_in_box(int x, int y) {
    return (y <= 17 && y >= -17) && (x <= 63 && x >= -64);
}