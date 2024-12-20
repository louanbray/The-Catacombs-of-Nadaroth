#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stdbool.h>

typedef struct element element;
typedef struct dynarray dynarray;

dynarray* create_dyn();
dynarray* create_dyn_from(int len, element** a);
int len_dyn(dynarray* t);
element* get_dyn(dynarray* t, int i);
void set_dyn(dynarray* t, int i, element* e);
void append(dynarray* t, element* e);
element* pop(dynarray* t);
void print_dyn(dynarray* t);
void free_dyn(dynarray* d);

#endif
