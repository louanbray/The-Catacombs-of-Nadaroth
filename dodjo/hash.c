#include "hash.h"

typedef struct list {
    int x, y;
    chunk* ck;
    struct list* next;
} list;

typedef struct hash_map_s {
    int max_collide;
    int length;
    int nb_e;
    list** hash_map;
} hm;

int ipow(int base, int exp) {
    int result = 1;
    for (;;) {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}

int hash(int h, int x, int y) {
    return (ipow(2, x) * ipow(3, y)) % h;
}

hm* create() {
    hm* ht = malloc(sizeof(hm));
    ht->length = 100;
    ht->nb_e = 0;
    ht->hash_map = malloc(sizeof(list*) * ht->length);
    ht->max_collide = 5;
    return ht;
}

int size(hm* t) {
    return t->nb_e;
}

int len(hm* t) {
    return t->length;
}

chunk get(hm* t, int x, int y) {
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

void setCell(list* cell, int x, int y, chunk* val) {
    cell->x = x;
    cell->y = y;
    cell->ck = val;
    cell->next = NULL;
};

bool check_resize(hm* t, list* l) {
    int i = 1;
    while (l != NULL) {
        l = l->next;
        i++;
    }
    return i > t->max_collide;
}

void resize(hm* t) {
    list** old = t->hash_map;
    int len = t->length;
    list** n_hm = malloc(sizeof(list*) * len * 2);
    t->hash_map = n_hm;
    t->length = len * 2;
    for (int i = 0; i < len; i++) {
        list* l = old[i];
        while (l != NULL) {
            set(t, l->x, l->y, l->ck);
            l = l->next;
        }
    }
    free(old);
}

void set(hm* t, int x, int y, chunk e) {
    int index = hash(t, x, y);
    list* l = malloc(sizeof(list));

    setCell(l, x, y, e);

    if (t->hash_map[index] == NULL) {
        t->hash_map[index] = l;
    } else {
        l->next = t->hash_map[index];
        t->hash_map[index] = l;
        if (check_resize(t, l->next)) {
            resize(t);
        }
    }
    t->nb_e += 1;
}

void remove(hm* t, int x, int y) {
    int index = hash(t, x, y);
    list* prev = NULL;
    list* curr = t->hash_map[index];

    while (curr != NULL) {
        if (x == curr->x && y == curr->y) {
            if (curr == t->hash_map[index]) {
                t->hash_map[index] = curr->next;
            } else {
                prev->next = curr->next;
            }
            free(curr);
            t->nb_e += -1;
            break;
        }
        prev = curr;
        curr = curr->next;
    }
}

void free_hm(hm* t) {
    for (int i = 0; i < t->length; i++) {
        list* l = t->hash_map[i];
        while (l != NULL) {
            list* t = l;
            l = l->next;
            free(t);
        }
    }
    free(t->hash_map);
    free(t);
}

void print_hm(hm* t) {
    for (int i = 0; i < t->length; i++) {
        list* l = t->hash_map[i];
        printf("hash: %d [", i);
        while (l != NULL) {
            printf("x: %d, y: %d, chunk: %p | ", i, l->x, l->y, l->ck);
            l = l->next;
        }
        printf("]\n");
    }
}