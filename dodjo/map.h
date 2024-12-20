
#ifndef MAP_H
#define MAP_H

typedef struct map map;
typedef struct chunk chunk;
typedef struct dynarray dynarray;
typedef chunk** link;

map* create_map();
chunk* getChunk(map* m, int x, int y);
chunk* getChunkFrom(map* m, chunk* c1, int dir);
void destroy_chunk(map* m, chunk* ck);
void print_chunk(chunk* ck);
void print_map(map* m);
chunk* get_spawn(map* m);
player* get_player(map* m);
dynarray* get_chunk_furniture(chunk* ck);
void set_chunk_type(chunk* c, int type);
#endif
