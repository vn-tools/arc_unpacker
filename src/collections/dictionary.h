#ifndef DICTIONARY_H
#define DICTIONARY_H
#include <stdbool.h>

typedef struct Dictionary Dictionary;

Dictionary *dictionary_create();

void dictionary_destroy(Dictionary *dictionary);

void dictionary_set(
    Dictionary *const dictionary,
    const char *const key,
    const void *const value);

const void *dictionary_get(
    const Dictionary *const dictionary,
    const char *const key);

bool dictionary_has_key(
    const Dictionary *const dictionary,
    const char *const key);

#endif
