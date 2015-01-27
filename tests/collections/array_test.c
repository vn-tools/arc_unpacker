#include "collections/array.h"
#include "assert.h"

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
    array_set(arr, 0, "test");
    assert_equals("test", array_get(arr, 0));
    array_destroy(arr);
}

void test_two_items()
{
    Array *arr = array_create();
    assert_null(array_get(arr, 0));
    array_set(arr, 0, "test1");
    array_set(arr, 1, "test2");
    assert_equals("test1", array_get(arr, 0));
    assert_equals("test2", array_get(arr, 1));
    array_destroy(arr);
}

void test_gaps()
{
    size_t i;
    Array *arr = array_create();
    assert_null(array_get(arr, 0));
    array_set(arr, 0, "test1");
    array_set(arr, 10240, "test2");
    assert_equals("test1", array_get(arr, 0));
    assert_equals("test2", array_get(arr, 10240));
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
