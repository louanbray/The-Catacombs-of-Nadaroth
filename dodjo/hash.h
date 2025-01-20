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
/// @param max_length max length
/// @param key_x chunk x
/// @param key_y chunk y
/// @return hash within (0->h)
int hash(int max_length, int key_x, int key_y);

/// @brief Create empty hashmap
/// @return hashmap*
hm* create_hashmap();

/// @brief Get the max pool of hashmap
/// @param hashmap hashmap*
/// @return pool size
int len_hm(hm* hashmap);

/// @brief Get the number of entries in hashmap @t
/// @param hashmap hashmap*
/// @return entries number of hashmap
int size_hm(hm* hashmap);

/// @brief Get value from hashmap with keys
/// @param hashmap hashmap*
/// @param key_x key1
/// @param key_y key2
/// @return NULL if empty else matching value
element_h get_hm(hm* hashmap, int key_x, int key_y);

/// @brief Set value from hashmap with keys
/// @param hashmap hashmap*
/// @param key_x key1
/// @param key_y key2
/// @param element element to insert
void set_hm(hm* hashmap, int key_x, int key_y, element_h element);

/// @brief Remove element linked to the keys
/// @param hashmap hashmap*
/// @param key_x key1
/// @param key_y key2
void purge_hm(hm* hashmap, int key_x, int key_y);

/// @brief Free the whole hashmap (w\ the element content if complex structure)
/// @param hashmap
void free_hm(hm* hashmap);

/// @brief Print hashmap @t in a user friendly way
/// @param hashmap hashmap*
void print_hm(hm* hashmap);

#endif
