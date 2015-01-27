#include "collections/dictionary.h"
#include "assert.h"

void test_non_existent_key()
{
    Dictionary *dic = dictionary_create();
    assert_null(dictionary_get(dic, "nope"));
    assert_that(!dictionary_has_key(dic, "nope"));
    dictionary_destroy(dic);
}

void test_one_key()
{
    int value = 5;
    Dictionary *dic = dictionary_create();
    dictionary_set(dic, "key", &value);
    assert_equalp(&value, dictionary_get(dic, "key"));
    assert_that(dictionary_has_key(dic, "key"));
    dictionary_destroy(dic);
}

void test_two_keys()
{
    int value1 = 5;
    int value2 = 6;
    Dictionary *dic = dictionary_create();
    dictionary_set(dic, "key1", &value1);
    dictionary_set(dic, "key2", &value2);
    assert_equalp(&value1, dictionary_get(dic, "key1"));
    assert_equalp(&value2, dictionary_get(dic, "key2"));
    assert_that(dictionary_has_key(dic, "key1"));
    assert_that(dictionary_has_key(dic, "key2"));
    dictionary_destroy(dic);
}

void test_two_keys_reverse_order()
{
    int value1 = 5;
    int value2 = 6;
    Dictionary *dic = dictionary_create();
    dictionary_set(dic, "key1", &value1);
    dictionary_set(dic, "key2", &value2);
    assert_equalp(&value2, dictionary_get(dic, "key2"));
    assert_equalp(&value1, dictionary_get(dic, "key1"));
    dictionary_destroy(dic);
}

int main(void)
{
    test_non_existent_key();
    test_one_key();
    test_two_keys();
    test_two_keys_reverse_order();
    return 0;
}
