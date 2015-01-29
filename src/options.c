#include "options.h"

Options *options_create()
{
    Options *options = dictionary_create();
    return options;
}

void options_destroy(Options *options)
{
    dictionary_destroy(options);
}

void options_set(
    Options *options,
    const char *key,
    const char *value)
{
    dictionary_set(options, key, (void*)value);
}

const char *options_get(
    const Options *options,
    const char *key)
{
    return (const char*)dictionary_get(options, key);
}
