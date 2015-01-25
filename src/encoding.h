#ifndef ENCODING_H
#define ENCODING_H

int convert(
    const char *input,
    size_t input_size,
    char **output,
    size_t *output_size,
    const char *const from,
    const char *const to);

#endif
