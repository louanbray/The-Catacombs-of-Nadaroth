
#ifndef MAP_H
#define MAP_H

typedef struct map map;
typedef struct chunk chunk;
typedef struct element element;
typedef chunk** link;

map* create_map();
chunk* getChunk(map* m, int x, int y);
chunk* getChunkFrom(map* m, chunk* c1, int dir);
void destroy_chunk(map* m, chunk* ck);
void print_chunk(chunk* ck);
void print_map(map* m);
#endif
