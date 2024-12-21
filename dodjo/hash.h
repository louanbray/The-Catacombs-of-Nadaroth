#ifndef HASH_H
#define HASH_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

/// @brief Hashmap
typedef struct hash_map_s hm;
/// @brief Contained element
typedef void* element_h;

/// @brief Hash function
/// @param h max length
/// @param x chunk x
/// @param y chunk y
/// @return hash within (0->h)
int hash(int h, int x, int y);

/// @brief Create empty hashmap
/// @return hashmap*
hm* create_hashmap();

/// @brief Get the max pool of hashmap
/// @param t hashmap*
/// @return pool size
int len_hm(hm* t);

/// @brief Get the number of entries in hashmap @t
/// @param t hashmap*
/// @return entries number of hashmap
int size_hm(hm* t);

/// @brief Get value from hashmap with keys
/// @param t hashmap*
/// @param x key1
/// @param y key2
/// @return NULL if empty else matching value
element_h get_hm(hm* t, int x, int y);

/// @brief Set value from hashmap with keys
/// @param t hashmap*
/// @param x key1
/// @param y key2
/// @param e element to insert
void set_hm(hm* t, int x, int y, element_h e);

/// @brief Remove element linked to the keys
/// @param t hashmap*
/// @param x key1
/// @param y key2
void purge_hm(hm* t, int x, int y);

/// @brief Free the whole hashmap (w\ the element content if complex structure)
/// @param t
void free_hm(hm* t);

/// @brief Print hashmap @t in a user friendly way
/// @param t hashmap*
void print_hm(hm* t);

#endif
