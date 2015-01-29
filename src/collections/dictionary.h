#ifndef DICTIONARY_H
#define DICTIONARY_H
#include <stdbool.h>
#include "collections/array.h"

typedef struct Dictionary Dictionary;

Dictionary *dictionary_create();

void dictionary_destroy(Dictionary *dictionary);

void dictionary_set(
    Dictionary *dictionary,
    const char *key,
    const void *value);

const Array *dictionary_get_keys(const Dictionary *dictionary);
const Array *dictionary_get_values(const Dictionary *dictionary);

const void *dictionary_get(
    const Dictionary *dictionary,
    const char *key);

bool dictionary_has_key(
    const Dictionary *dictionary,
    const char *key);

#endif
