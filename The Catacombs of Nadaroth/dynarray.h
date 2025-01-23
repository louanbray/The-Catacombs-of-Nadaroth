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
/// @param elements array of elements
/// @return dynarray
dynarray* create_dyn_from(int len, element* elements);

/// @brief Return the number of element in the array
/// @param array dynarray
/// @return length
int len_dyn(dynarray* array);

/// @brief Get element of index i
/// @param array dynarray
/// @param index index
/// @return element
element get_dyn(dynarray* array, int index);

/// @brief Set element of index i to e
/// @param array dynarray
/// @param index index
/// @param element element
void set_dyn(dynarray* array, int index, element element);

/// @brief Add element e to the dynarray
/// @param array dynarray
/// @param element element
void append(dynarray* array, element element);

/// @brief Remove last added element
/// @param array dynarray
/// @return destroyed element
element pop(dynarray* array);

/// @brief Print in a user friendly way the dynarray
/// @param array dynarray
void print_dyn(dynarray* array);

/// @brief Free memory
/// @param array dynarray
void free_dyn(dynarray* array);

#endif
