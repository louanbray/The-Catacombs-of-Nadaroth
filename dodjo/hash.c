#include "hash.h"

#include <stdio.h>
#include <time.h>

typedef struct list {
    int x, y;
    chunk_h ck;
    struct list* next;
} list;

typedef struct hash_map_s {
    int max_collide;
    int length;
    int nb_e;
    list** hash_map;
} hm;

/// @brief Hash function
/// @param h max length
/// @param x chunk x
/// @param y chunk y
/// @return hash within (0->h)
int hash(int h, int x, int y) {
    const unsigned int FNV_prime = 16777619;
    unsigned int hash = 2166136261;

    hash = (hash ^ x) * FNV_prime;
    hash = (hash ^ y) * FNV_prime;

    return hash % h;
}

/// @brief Create empty hashmap
/// @return hashmap*
hm* create_hashmap() {
    hm* ht = malloc(sizeof(hm));
    ht->length = 100;
    ht->nb_e = 0;
    ht->hash_map = calloc(ht->length, sizeof(list*));
    ht->max_collide = 5;
    return ht;
}

/// @brief Get the number of entries in hashmap @t
/// @param t hashmap*
/// @return entries number of hashmap
int size_hm(hm* t) {
    return t->nb_e;
}

/// @brief Get the max pool of hashmap
/// @param t hashmap*
/// @return pool size
int len_hm(hm* t) {
    return t->length;
}

/// @brief Get value from hashmap with keys
/// @param t hashmap*
/// @param x chunk x
/// @param y chunk y
/// @return NULL if empty else matching value
chunk_h get_hm(hm* t, int x, int y) {
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

/// @brief Used to update bucket list
/// @param cell bucketlist cell
/// @param x chunk x
/// @param y chunk y
/// @param val chunk_h (value)
void setCell(list* cell, int x, int y, chunk_h val) {
    cell->x = x;
    cell->y = y;
    cell->ck = val;
    cell->next = NULL;
}

/// @brief Check if resize needed (collisions in a bucket > max_collisions)
/// @param t hashmap*
/// @param l current bucketlist
/// @return true if resize needed
bool check_resize(hm* t, list* l) {
    int i = 0;
    while (l != NULL) {
        l = l->next;
        i++;
    }
    return i > t->max_collide;
}

/// @brief Resize hashmap with 2* pool length
/// @param t hashmap*
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
            free(k);
        }
    }
    free(old);
}

void set_hm(hm* t, int x, int y, chunk_h e) {
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

void purge_hm(hm* t, int x, int y) {
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

// int main() {
//     hm* t = create_hashmap();
//     srand(time(NULL));
//     for (int i = 0; i < 100000; i++) {
//         set_hm(t, (rand() % 100000), (rand() % 100000), NULL);
//     }
//     print_hm(t);
// }