#ifndef ARRAY_H
#define ARRAY_H
#include <stddef.h>
#include <stdbool.h>

typedef struct Array Array;

Array *array_create();

void array_destroy(Array *array);

size_t array_size(Array *array);

void *array_get(Array *array, size_t index);

bool array_set(Array *array, size_t index, void *data);

#endif
