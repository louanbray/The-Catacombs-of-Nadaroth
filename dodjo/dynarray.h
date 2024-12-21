#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

/// @brief Contained element
typedef void* element;
/// @brief Dynarray
typedef struct dynarray dynarray;

/// @brief Create an empty dynarray
/// @return dynarray
dynarray* create_dyn();

/// @brief Create a dynarray containing the given set of elements a of length len
/// @param len length of array
/// @param a array of elements
/// @return dynarray
dynarray* create_dyn_from(int len, element* a);

/// @brief Return the number of element in the array
/// @param t dynarray
/// @return length
int len_dyn(dynarray* t);

/// @brief Get element of index i
/// @param t dynarray
/// @param i index
/// @return element
element get_dyn(dynarray* t, int i);

/// @brief Set element of index i to e
/// @param t dynarray
/// @param i index
/// @param e element
void set_dyn(dynarray* t, int i, element e);

/// @brief Add element e to the dynarray
/// @param t dynarray
/// @param e element
void append(dynarray* t, element e);

/// @brief Remove last added element
/// @param t dynarray
/// @return destroyed element
element pop(dynarray* t);

/// @brief Print in a user friendly way the dynarray
/// @param t dynarray
void print_dyn(dynarray* t);

/// @brief Free memory
/// @param d dynarray
void free_dyn(dynarray* d);

#endif
