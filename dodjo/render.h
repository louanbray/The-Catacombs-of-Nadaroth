
#ifndef RENDER_H
#define RENDER_H

typedef struct inventory inventory;
typedef struct dynarray dynarray;
typedef struct player player;
typedef struct chunk chunk;
typedef struct map map;

typedef char** board;

board new_screen();
void render_elements(board b, dynarray* c);
void render_chunk(board b, chunk* c);
void render_player(board b, player* p);
void render_inventory(board b, inventory* i);
void render(board b, map* map);
void update_screen(board b);

#endif
