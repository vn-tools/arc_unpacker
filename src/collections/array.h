#ifndef COLLECTIONS_ARRAY_H
#define COLLECTIONS_ARRAY_H
#include <stdbool.h>
#include <stddef.h>

typedef struct Array Array;

Array *array_create();

void array_destroy(Array *array);

size_t array_size(const Array *array);

void *array_get(const Array *array, size_t index);

bool array_set(Array *array, size_t index, void *data);

bool array_add(Array *array, void *data);

#endif
