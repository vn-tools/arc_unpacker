#include <cassert>
#include <cstdlib>
#include <cstring>
#include "array.h"

struct Array
{
    size_t size;
    void **items;
};

Array *array_create()
{
    Array *array = new Array;
    assert(array != NULL);
    array->size = 0;
    array->items = NULL;
    return array;
}

void array_destroy(Array *array)
{
    assert(array != NULL);
    free(array->items);
    delete array;
}

size_t array_size(const Array *array)
{
    assert(array != NULL);
    return array->size;
}

void *array_get(const Array *array, size_t index)
{
    assert(array != NULL);
    if (index >= array->size)
        return NULL;
    return array->items[index];
}

bool array_set(Array *array, size_t index, void *data)
{
    void **new_items;
    size_t new_size;
    assert(array != NULL);
    if (index >= array->size)
    {
        new_size = index + 1;
        new_items = (void**)realloc(array->items, sizeof(void*) * new_size);
        if (!new_items)
            return false;
        array->items = new_items;
        memset(array->items + array->size, 0, new_size - 1 - array->size);
        array->size = new_size;
    }
    array->items[index] = data;
    return true;
}

bool array_add(Array *array, void *data)
{
    return array_set(array, array_size(array), data);
}
