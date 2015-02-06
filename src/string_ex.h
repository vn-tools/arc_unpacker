#ifndef STRING_EX_H
#define STRING_EX_H
#include <stdbool.h>
#include <stddef.h>

bool convert_encoding(
    const char *input,
    size_t input_size,
    char **output,
    size_t *output_size,
    const char *from,
    const char *to);

bool zlib_inflate(
    const char *input,
    size_t input_size,
    char **output,
    size_t *output_size);

#endif
