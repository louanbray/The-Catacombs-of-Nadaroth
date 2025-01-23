#include "handler.h"

#include <stdio.h>

#include "entity.h"
#include "generation.h"

/// @brief Return the part of the map the player is in as a direction (USED ONLY FOR GATES)
/// @param x player x
/// @param y player y
/// @return int direction
int get_direction(int x, int y) {
    if (x > 10) {
        return EAST;
    } else if (x < -10) {
        return WEST;
    } else if (y > 10) {
        return NORTH;
    } else if (y < -10) {
        return SOUTH;
    }
    return 0;
}

int pickup_from_chunk(player* p, item* i) {
    if (is_hotbar_full(get_player_hotbar(p))) return 2;

    int isAnEntity = is_an_entity(i);

    if (isAnEntity) {
        entity* e = get_entity_link(i);

        i = get_entity_brain(e);

        remove_entity_from_chunk(e);
    } else {
        remove_item(get_player_chunk(p), i);
    }

    pickup(get_player_hotbar(p), i);

    return 3 - isAnEntity * 2;
}

int handle(player* p, int x, int y) {
    chunk* ck = get_player_chunk(p);
    hm* h = get_chunk_furniture_coords(ck);

    item* i = get_hm(h, x, y);

    if (i != NULL) {
        if (is_item_hidden(i)) return 0;

        switch (get_item_type(i)) {  //? Modify to an item-player interaction
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