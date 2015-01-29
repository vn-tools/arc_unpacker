#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "assert.h"

struct Array
{
    size_t size;
    void **items;
};

Array *array_create()
{
    Array *array = malloc(sizeof(Array));
    assert_not_null(array);
    array->size = 0;
    array->items = NULL;
    return array;
}

void array_destroy(Array *array)
{
    assert_not_null(array);
    free(array->items);
    free(array);
}

size_t array_size(Array *array)
{
    assert_not_null(array);
    return array->size;
}

void *array_get(Array *array, size_t index)
{
    assert_not_null(array);
    if (index >= array->size)
        return NULL;
    return array->items[index];
}

bool array_set(Array *array, size_t index, void *data)
{
    void *new_items;
    size_t new_size;
    assert_not_null(array);
    if (index >= array->size)
    {
        new_size = index + 1;
        new_items = realloc(array->items, sizeof(void*) * new_size);
        if (!new_items)
            return false;
        array->items = new_items;
        memset(array->items + array->size, 0, new_size - 1 - array->size);
        array->size = new_size;
    }
    array->items[index] = data;
    return true;
}
