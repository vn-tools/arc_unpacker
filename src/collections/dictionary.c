#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "collections/dictionary.h"
#include "collections/linked_list.h"

// super stupid implementation with O(n) lookup time and O(n) insertion time

struct Dictionary
{
    Array *keys;
    Array *values;
};

static ssize_t get_index_for_key(const Dictionary *dictionary, const char *key)
{
    size_t i;
    for (i = 0; i < array_size(dictionary->keys); i ++)
    {
        if (strcmp((const char*)array_get(dictionary->keys, i), key) == 0)
        {
            return i;
        }
    }
    return -1;
}

Dictionary *dictionary_create()
{
    Dictionary *dictionary = (Dictionary*)malloc(sizeof(Dictionary));
    assert(dictionary != NULL);
    dictionary->values = array_create();
    dictionary->keys = array_create();
    return dictionary;
}

void dictionary_destroy(Dictionary *dictionary)
{
    assert(dictionary != NULL);
    array_destroy(dictionary->keys);
    array_destroy(dictionary->values);
    free(dictionary);
}

const Array *dictionary_get_keys(const Dictionary *const dictionary)
{
    return dictionary->keys;
}

const Array *dictionary_get_values(const Dictionary *const dictionary)
{
    return dictionary->values;
}

void dictionary_set(
    Dictionary *dictionary,
    const char *key,
    const void *value)
{
    ssize_t i = get_index_for_key(dictionary, key);
    if (i == -1)
    {
        i = array_size(dictionary->keys);
        if (!array_set(dictionary->keys, i, (void*)key))
            assert(0);
    }
    if (!array_set(dictionary->values, i, (void*)value))
        assert(0);
}

const void *dictionary_get(
    const Dictionary *const dictionary,
    const char *const key)
{
    ssize_t i = get_index_for_key(dictionary, key);
    if (i == -1)
        return NULL;
    return array_get(dictionary->values, i);
}

bool dictionary_has_key(
    const Dictionary *const dictionary,
    const char *const key)
{
    return get_index_for_key(dictionary, key) != -1;
}
