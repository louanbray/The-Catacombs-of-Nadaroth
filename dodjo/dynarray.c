#include "dynarray.h"

#include <stdio.h>

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
    printf("len : %d, memlen : %d, elements : [", t->len, t->memlen);
    for (int i = 0; i < t->len; i++) {
        printf("%p - ", t->elt[i]);
    }
    printf("]\n");
}

dynarray* create_dyn() {
    dynarray* da = malloc(sizeof(dynarray));
    element* elt = malloc(sizeof(element) * 10);
    da->elt = elt;
    da->len = 0;
    da->memlen = 10;
    return da;
}

dynarray* create_dyn_from(int len, element* a) {
    dynarray* da = create_dyn();
    da->len = len;
    da->memlen = len;
    da->elt = a;
    return da;
}

int len_dyn(dynarray* t) {
    return t->len;
}

element get_dyn(dynarray* t, int i) {
    return t->elt[i];
}

void set_dyn(dynarray* t, int i, element e) {
    t->elt[i] = e;
}

/// @brief Resize the dynarray to asked size
/// @param t dynarray
/// @param newlen New length
void resize_dyn(dynarray* t, int newlen) {
    element* elt = malloc(sizeof(element) * newlen);
    for (int i = 0; i < t->len; i++) {
        elt[i] = t->elt[i];
    }
    free(t->elt);
    t->elt = elt;
    t->memlen = newlen;
}

void append(dynarray* t, element e) {
    if (t->memlen == t->len) {
        resize_dyn(t, t->memlen * 2);
    }
    t->elt[t->len] = e;
    t->len += 1;
}

element pop(dynarray* t) {
    assert(t->len != 0);
    element r = t->elt[t->len - 1];
    if (t->memlen > 10 && t->memlen > 1 + 4 * (t->len)) {
        resize_dyn(t, max(10, t->memlen / 2));
    }
    t->len += -1;
    return r;
}

void free_dyn(dynarray* t) {
    if (t != NULL) {
        int ln = len_dyn(t);
        for (int i = 0; i < ln; i++) {
            if (t->elt[i] != NULL) {
                free(t->elt[i]);
            }
        }
        free(t->elt);
        free(t);
    }
}
