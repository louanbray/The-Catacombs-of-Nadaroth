#include "render.h"

#include "dynarray.h"
#include "map.h"
// #include "player.h"
//  #include "inventory.h"

#define clear_screen() printf("\033[H\033[J")

const int RENDER_WIDTH = 130;
const int RENDER_HEIGHT = 40;

board new_screen() {
    board b = malloc(sizeof(char*) * RENDER_HEIGHT);
    for (int i = 0; i < RENDER_HEIGHT; i++) {
        char* row = malloc(RENDER_WIDTH, sizeof(char));
        for (int j = 0; j < RENDER_WIDTH; j++) {
            row[j] = ' ';
        }
        b[i] = row;
    }
    return b;
}

void render_elements(board b, dynarray* c);
void render_chunk(board b, chunk* c);
void render_player(board b, player* p);
void render_inventory(board b, inventory* i);
void render(board b, map* map) {
    player* player = get_player(map);
    chunk* curr = get_current_chunk(player);
    render_chunk(b, curr);
    render_elements(b, get_chunk_furniture(curr));
    render_player(b, player);
    render_inventory(b, get_inventory(player));
}

void update_screen(board b) {
    clear_screen();
    for (int i = RENDER_HEIGHT - 1; i >= 0; i--) {
        char buffer[RENDER_WIDTH + 1];
        sprintf(&buffer[0], "%130s", &b[i][0]);
        printf("%130s\n", buffer);
    }
}