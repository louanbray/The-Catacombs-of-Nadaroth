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

bool handle(player* p, int x, int y) {
    chunk* ck = get_player_chunk(p);
    hm* h = get_chunk_furniture_coords(ck);
    item* i = get_hm(h, x, y);
    if (i != NULL) {
        switch (get_item_type(i)) {
            case GATE:
                move_player_chunk(p, get_direction(x, y));
                return false;
                break;
            case SGATE:
                move_player_chunk(p, STARGATE);
                return false;
                break;
            default:
                return false;
                break;
        }
    }
    return true;
}

bool is_in_box(int x, int y) {
    return (y <= 17 && y >= -17) && (x <= 63 && x >= -64);
}