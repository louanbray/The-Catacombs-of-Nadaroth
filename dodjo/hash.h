#ifndef HASH_H
#define HASH_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct hash_map_s hm;
typedef void* chunk_h;

int hash(int h, int x, int y);
hm* create_hashmap();
int len_hm(hm* t);
int size_hm(hm* t);
chunk_h get_hm(hm* t, int x, int y);
void set_hm(hm* t, int x, int y, chunk_h e);
void purge_hm(hm* t, int x, int y);
void free_hm(hm* t);
void print_hm(hm* t);

#endif
