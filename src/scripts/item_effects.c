#include "item_effects.h"

#include "../game_objects/player.h"

typedef bool (*ItemUseCallback)(player* p);

bool effect_golden_apple(player* p) {
    set_player_max_health(p, get_player_max_health(p) + 1);
    return true;
}

bool effect_onion_ring(player* p) {
    if (get_player_max_health(p) != get_player_health(p)) {
        heal_player(p, 1);
        return true;
    }
    return false;
}

bool effect_stockfish(player* p) {
    if (get_player_max_health(p) != get_player_health(p)) {
        heal_player(p, get_player_max_health(p) - get_player_health(p));
        return true;
    }
    return false;
}

bool effect_bomb(player* p) {
    destroy_player_cchunk(p);
    return true;
}

bool effect_school_dishes(player* p) {
    if (get_player_mental_health(p) != 4) {
        modify_player_mental_health(p, 1);
        return true;
    }
    return false;
}

bool effect_forgotten_dish(player* p) {
    if (get_player_mental_health(p) != 4) {
        set_player_mental_health(p, 4);
        return true;
    }
    return false;
}

static const ItemUseCallback ITEM_EFFECTS[USABLE_ITEM_COUNT] = {
    [USABLE_ITEM_NOT_USABLE] = NULL,
    [USABLE_ITEM_GOLDEN_APPLE] = effect_golden_apple,
    [USABLE_ITEM_STOCKFISH] = effect_stockfish,
    [USABLE_ITEM_ONION_RING] = effect_onion_ring,
    [USABLE_ITEM_BOMB] = effect_bomb,
    [USABLE_ITEM_FORGOTTEN_DISH] = effect_forgotten_dish,
};

bool use_item(UsableItem type, player* p) {
    if (type < USABLE_ITEM_NONE || type >= USABLE_ITEM_COUNT) return false;
    ItemUseCallback effect = ITEM_EFFECTS[type];

    if (effect != NULL) return effect(p);
    return false;
}