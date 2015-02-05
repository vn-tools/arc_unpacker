#include <assert.h>
#include <string.h>
#include "collections/array.h"

void test_empty_array()
{
    Array *arr = array_create();
    assert(array_get(arr, 0) == nullptr);
    assert(0 == array_size(arr));
    array_destroy(arr);
}

void test_one_item()
{
    Array *arr = array_create();
    assert(array_get(arr, 0) == nullptr);
    array_set(arr, 0, (void*)"test");
    assert(strcmp("test", (const char*)array_get(arr, 0)) == 0);
    array_destroy(arr);
}

void test_two_items()
{
    Array *arr = array_create();
    assert(array_get(arr, 0) == nullptr);
    array_set(arr, 0, (void*)"test1");
    array_set(arr, 1, (void*)"test2");
    assert(strcmp("test1", (const char*)array_get(arr, 0)) == 0);
    assert(strcmp("test2", (const char*)array_get(arr, 1)) == 0);
    array_destroy(arr);
}

void test_gaps()
{
    size_t i;
    Array *arr = array_create();
    assert(array_get(arr, 0) == nullptr);
    array_set(arr, 0, (void*)"test1");
    array_set(arr, 10240, (void*)"test2");
    assert(strcmp("test1", (const char*)array_get(arr, 0)) == 0);
    assert(strcmp("test2", (const char*)array_get(arr, 10240)) == 0);
    for (i = 1; i < 10240; i ++)
        assert(array_get(arr, i) == nullptr);
    array_destroy(arr);
}

void test_add()
{
    Array *arr = array_create();
    array_add(arr, (void*)"test1");
    array_add(arr, (void*)"test2");
    assert(strcmp("test1", (const char*)array_get(arr, 0)) == 0);
    assert(strcmp("test2", (const char*)array_get(arr, 1)) == 0);
    array_destroy(arr);
}

int main(void)
{
    test_empty_array();
    test_one_item();
    test_two_items();
    test_gaps();
    test_add();
    return 0;
}
