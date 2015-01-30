#include "assert_ex.h"
#include "collections/array.h"

void test_empty_array()
{
    Array *arr = array_create();
    assert_null(array_get(arr, 0));
    assert_equali(0, array_size(arr));
    array_destroy(arr);
}

void test_one_item()
{
    Array *arr = array_create();
    assert_null(array_get(arr, 0));
    array_set(arr, 0, (void*)"test");
    assert_equals("test", (const char*)array_get(arr, 0));
    array_destroy(arr);
}

void test_two_items()
{
    Array *arr = array_create();
    assert_null(array_get(arr, 0));
    array_set(arr, 0, (void*)"test1");
    array_set(arr, 1, (void*)"test2");
    assert_equals("test1", (const char*)array_get(arr, 0));
    assert_equals("test2", (const char*)array_get(arr, 1));
    array_destroy(arr);
}

void test_gaps()
{
    size_t i;
    Array *arr = array_create();
    assert_null(array_get(arr, 0));
    array_set(arr, 0, (void*)"test1");
    array_set(arr, 10240, (void*)"test2");
    assert_equals("test1", (const char*)array_get(arr, 0));
    assert_equals("test2", (const char*)array_get(arr, 10240));
    for (i = 1; i < 10240; i ++)
        assert_null(array_get(arr, i));
    array_destroy(arr);
}

int main(void)
{
    test_empty_array();
    test_one_item();
    test_two_items();
    test_gaps();
    return 0;
}
