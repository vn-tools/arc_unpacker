#ifndef OPTIONS_H
#define OPTIONS_H
#include "collections/dictionary.h"

// TODO:
// this is a straightforward port of "options" hash from Ruby version.
// This should be replaced with a solution that is more fitting for C version,
// i.e. a dedicated struct.

typedef Dictionary Options;

Options *options_create();

void options_destroy(Options *options);

void options_set(
    Options *options,
    const char *key,
    const char *value);

const char *options_get(
    const Options *options,
    const char *key);

#endif
