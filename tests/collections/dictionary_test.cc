#include <assert.h>
#include <string.h>
#include "collections/dictionary.h"

void test_non_existent_key()
{
    Dictionary *dic = dictionary_create();
    assert(dictionary_get(dic, "nope") == nullptr);
    assert(!dictionary_has_key(dic, "nope"));
    dictionary_destroy(dic);
}

void test_one_key()
{
    int value = 5;
    Dictionary *dic = dictionary_create();
    dictionary_set(dic, "key", &value);
    assert(&value == dictionary_get(dic, "key"));
    assert(dictionary_has_key(dic, "key"));
    dictionary_destroy(dic);
}

void test_two_keys()
{
    int value1 = 5;
    int value2 = 6;
    Dictionary *dic = dictionary_create();
    dictionary_set(dic, "key1", &value1);
    dictionary_set(dic, "key2", &value2);
    assert(&value1 == dictionary_get(dic, "key1"));
    assert(&value2 == dictionary_get(dic, "key2"));
    assert(dictionary_has_key(dic, "key1"));
    assert(dictionary_has_key(dic, "key2"));
    dictionary_destroy(dic);
}

void test_two_keys_reverse_order()
{
    int value1 = 5;
    int value2 = 6;
    Dictionary *dic = dictionary_create();
    dictionary_set(dic, "key1", &value1);
    dictionary_set(dic, "key2", &value2);
    assert(&value2 == (int*)dictionary_get(dic, "key2"));
    assert(&value1 == (int*)dictionary_get(dic, "key1"));
    dictionary_destroy(dic);
}

void test_all_keys()
{
    int value1 = 5;
    int value2 = 6;
    const Array *keys;
    Dictionary *dic = dictionary_create();
    dictionary_set(dic, "key1", &value1);
    dictionary_set(dic, "key2", &value2);
    keys = dictionary_get_keys(dic);
    assert(2 == array_size(keys));
    assert(strcmp("key1", (const char*)array_get(keys, 0)) == 0);
    assert(strcmp("key2", (const char*)array_get(keys, 1)) == 0);
    dictionary_destroy(dic);
}

void test_all_values()
{
    int value1 = 5;
    int value2 = 6;
    const Array *values;
    Dictionary *dic = dictionary_create();
    dictionary_set(dic, "key1", &value1);
    dictionary_set(dic, "key2", &value2);
    values = dictionary_get_values(dic);
    assert(2 == array_size(values));
    assert(&value1 == (int*)array_get(values, 0));
    assert(&value2 == (int*)array_get(values, 1));
    dictionary_destroy(dic);
}

int main(void)
{
    test_non_existent_key();
    test_one_key();
    test_two_keys();
    test_two_keys_reverse_order();
    test_all_keys();
    test_all_values();
    return 0;
}
