#include "hash.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/// @brief Bucket list
typedef struct list {
    int x, y;
    element_h ck;
    struct list* next;
} list;

typedef struct hash_map_s {
    int max_collide;
    int length;
    int nb_e;
    list** hash_map;
} hm;

int hash(int h, int x, int y) {
    const unsigned int FNV_prime = 16777619;
    unsigned int hash = 2166136261;

    hash = (hash ^ x) * FNV_prime;
    hash = (hash ^ y) * FNV_prime;

    return hash % h;
}

hm* create_hashmap() {
    hm* ht = (hm*)malloc(sizeof(hm));
    ht->length = 100;
    ht->nb_e = 0;
    ht->hash_map = (list**)calloc(ht->length, sizeof(list*));
    ht->max_collide = 5;
    return ht;
}

int size_hm(hm* t) {
    return t->nb_e;
}

int len_hm(hm* t) {
    return t->length;
}

element_h get_hm(hm* t, int x, int y) {
    int index = hash(t->length, x, y);
    list* hd = t->hash_map[index];
    while (hd != NULL) {
        if (hd->x == x && hd->y == y) {
            return hd->ck;
        }
        hd = hd->next;
    }
    return NULL;
}

/// @brief Set a cell in the bucket list
/// @param cell Bucket list cell
/// @param x X-coordinate
/// @param y Y-coordinate
/// @param val Element value
void setCell(list* cell, int x, int y, element_h val) {
    cell->x = x;
    cell->y = y;
    cell->ck = val;
    cell->next = NULL;
}

/// @brief Check if resizing is needed
/// @param t Hash map
/// @param l Current bucket list
/// @return True if resizing is needed
bool check_resize(hm* t, list* l) {
    int i = 0;
    while (l != NULL) {
        l = l->next;
        i++;
    }
    return i > t->max_collide;
}

/// @brief Resize the hash map to twice its capacity
/// @param t Hash map
void resize_hm(hm* t) {
    list** old = t->hash_map;
    int old_length = t->length;
    t->length = old_length * 2;
    t->hash_map = (list**)calloc(t->length, sizeof(list*));

    for (int i = 0; i < old_length; i++) {
        list* l = old[i];
        while (l != NULL) {
            int index = hash(t->length, l->x, l->y);
            list* l2 = (list*)malloc(sizeof(list));
            setCell(l2, l->x, l->y, l->ck);
            l2->next = t->hash_map[index];
            t->hash_map[index] = l2;

            list* temp = l;
            l = l->next;
            free(temp);
        }
    }
    free(old);
}

void set_hm(hm* t, int x, int y, element_h e) {
    int index = hash(t->length, x, y);
    list* l = (list*)malloc(sizeof(list));
    setCell(l, x, y, e);
    l->next = t->hash_map[index];
    t->hash_map[index] = l;

    if (check_resize(t, l)) {
        resize_hm(t);
    }
    t->nb_e++;
}

void purge_hm(hm* t, int x, int y) {
    int index = hash(t->length, x, y);
    list* prev = NULL;
    list* curr = t->hash_map[index];

    while (curr != NULL) {
        if (curr->x == x && curr->y == y) {
            if (prev == NULL) {
                t->hash_map[index] = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr);
            t->nb_e--;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void free_hm(hm* t) {
    for (int i = 0; i < t->length; i++) {
        list* l = t->hash_map[i];
        while (l != NULL) {
            list* temp = l;
            l = l->next;
            free(temp);
        }
    }
    free(t->hash_map);
    free(t);
}

void print_hm(hm* t) {
    for (int i = 0; i < t->length; i++) {
        list* l = t->hash_map[i];
        if (l != NULL) {
            printf("hash: %d, (len: %d) [", i, t->nb_e);
            while (l != NULL) {
                printf("x: %d, y: %d, chunk: %p | ", l->x, l->y, (void*)l->ck);
                l = l->next;
            }
            printf("]\n");
        }
    }
}
