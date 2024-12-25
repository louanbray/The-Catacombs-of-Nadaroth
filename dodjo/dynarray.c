#include "dynarray.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "item.h"

typedef struct dynarray {
    element* elt;
    int len;
    int memlen;
} dynarray;

/// @brief Return the max value between n1 and n2
/// @param n1 number to compare
/// @param n2 number to compare
/// @return max value
int max(int n1, int n2) {
    return n1 > n2 ? n1 : n2;
}

void print_dyn(dynarray* t) {
    printf("len: %d, memlen: %d, elements: [", t->len, t->memlen);
    for (int i = 0; i < t->len; i++) {
        printf("%p", (void*)t->elt[i]);
        if (i < t->len - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

dynarray* create_dyn() {
    dynarray* da = (dynarray*)malloc(sizeof(dynarray));
    da->elt = (element*)malloc(sizeof(element) * 10);
    da->len = 0;
    da->memlen = 10;
    return da;
}

/// @brief Resize the dynamic array to a new size
/// @param t dynarray
/// @param newlen New length
void resize_dyn(dynarray* t, int newlen) {
    assert(newlen > 0);
    element* elt = (element*)malloc(sizeof(element) * newlen);
    for (int i = 0; i < t->len; i++) {
        elt[i] = t->elt[i];
    }
    free(t->elt);
    t->elt = elt;
    t->memlen = newlen;
}

dynarray* create_dyn_from(int len, element* a) {
    dynarray* da = create_dyn();
    resize_dyn(da, len);
    da->len = len;
    for (int i = 0; i < len; i++) {
        da->elt[i] = a[i];
    }
    return da;
}

int len_dyn(dynarray* t) {
    return t->len;
}

element get_dyn(dynarray* t, int i) {
    assert(i >= 0 && i < t->len);
    return t->elt[i];
}

void set_dyn(dynarray* t, int i, element e) {
    assert(i >= 0 && i < t->len);
    t->elt[i] = e;
}

void append(dynarray* t, element e) {
    if (t->len == t->memlen) {
        resize_dyn(t, t->memlen * 2);
    }
    t->elt[t->len++] = e;
}

element pop(dynarray* t) {
    assert(t->len > 0);
    element r = t->elt[--t->len];
    if (t->memlen > 10 && t->memlen > 4 * t->len) {
        resize_dyn(t, max(10, t->memlen / 2));
    }
    return r;
}

void free_dyn(dynarray* t) {
    if (t) {
        for (int i = 0; i < t->len; i++) {
            if (t->elt[i]) {
                item* it = (item*)t->elt[i];
                free_item(it);
            }
        }
        free(t->elt);
        free(t);
    }
}
