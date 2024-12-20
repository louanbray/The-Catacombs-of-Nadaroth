#include "hash.h";

#include <stdio.h>
#include <time.h>

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

int hash(int h, int x, int y) {
    const unsigned int FNV_prime = 16777619;
    unsigned int hash = 2166136261;

    hash = (hash ^ x) * FNV_prime;
    hash = (hash ^ y) * FNV_prime;

    return hash % h;
}

hm* create() {
    hm* ht = malloc(sizeof(hm));
    ht->length = 100;
    ht->nb_e = 0;
    ht->hash_map = calloc(ht->length, sizeof(list*));
    ht->max_collide = 5;
    return ht;
}

int size(hm* t) {
    return t->nb_e;
}

int len(hm* t) {
    return t->length;
}

chunk* get(hm* t, int x, int y) {
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
}

bool check_resize(hm* t, list* l) {
    int i = 0;
    while (l != NULL) {
        l = l->next;
        i++;
    }
    return i > t->max_collide;
}

void resize(hm* t) {
    list** old = t->hash_map;
    int len = t->length;
    t->hash_map = calloc(len * 2, sizeof(list*));
    t->length = len * 2;
    for (int i = 0; i < len; i++) {
        list* l = old[i];
        while (l != NULL) {
            int index = hash(t->length, l->x, l->y);
            list* l2 = malloc(sizeof(list));

            setCell(l2, l->x, l->y, l->ck);
            l2->next = NULL;
            if (t->hash_map[index] == NULL) {
                t->hash_map[index] = l2;
            } else {
                l2->next = t->hash_map[index];
                t->hash_map[index] = l2;
            }
            l = l->next;
        }
    }
    for (int i = 0; i < len; i++) {
        list* l3 = old[i];
        while (l3 != NULL) {
            list* k = l3;
            l3 = l3->next;
            if (k->ck != NULL) {
                free(k->ck);
            }
            free(k);
        }
    }
    free(old);
}

void set(hm* t, int x, int y, chunk* e) {
    int index = hash(t->length, x, y);
    list* l = malloc(sizeof(list));

    setCell(l, x, y, e);
    l->next = NULL;
    if (t->hash_map[index] == NULL) {
        t->hash_map[index] = l;
    } else {
        l->next = t->hash_map[index];
        t->hash_map[index] = l;
        if (check_resize(t, l)) {
            resize(t);
        }
    }
    t->nb_e += 1;
}

void purge(hm* t, int x, int y) {
    int index = hash(t->length, x, y);
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
            list* k = l;
            l = l->next;
            if (k->ck != NULL) {
                free(k->ck);
            }
            free(k);
        }
    }
    free(t->hash_map);
    free(t);
}

void print_hm(hm* t) {
    for (int i = 0; i < t->length; i++) {
        list* l = t->hash_map[i];
        if (l != NULL) {
            printf("hash: %d [", i);
            while (l != NULL) {
                printf("x: %d, y: %d, chunk: %p | ", l->x, l->y, l->ck);
                l = l->next;
            }
            printf("]\n");
        }
    }
}
