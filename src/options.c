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
    const char *const key,
    const char *const value)
{
    dictionary_set(options, key, value);
}

const char *options_get(
    const Options *const options,
    const char *const key)
{
    return dictionary_get(options, key);
}
