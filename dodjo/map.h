
#ifndef MAP_H
#define MAP_H

typedef struct map map;
typedef struct chunk chunk;
typedef struct element element;
typedef struct link link;
enum Direction;

map* create_map();
chunk* getChunk(map* m, int x, int y);
chunk* getChunkFrom(map* m, chunk* c1, Direction dir);
void link(chunk* c1, chunk* c2, Direction dir);
void destroy_chunk(map* m, chunk* ck);

#endif
