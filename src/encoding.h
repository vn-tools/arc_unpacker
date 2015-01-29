#ifndef ENCODING_H
#define ENCODING_H
#include <stddef.h>
#include <stdbool.h>

bool convert(
    const char *input,
    size_t input_size,
    char **output,
    size_t *output_size,
    const char *from,
    const char *to);

#endif
