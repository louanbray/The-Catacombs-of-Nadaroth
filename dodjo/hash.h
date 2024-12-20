#ifndef HASH_H
#define HASH_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct hash_map_s hm;
typedef int chunk;

int hash(int h, int x, int y);
hm* create();
int len(hm* t);
int size(hm* t);
chunk* get(hm* t, int x, int y);
void set(hm* t, int x, int y, chunk* e);
void purge(hm* t, int x, int y);
void free_hm(hm* t);
void print_hm(hm* t);

#endif