#ifndef STRING_EX_H
#define STRING_EX_H
#include <cstddef>
#include <string>

bool convert_encoding(
    const char *input,
    size_t input_size,
    char **output,
    size_t *output_size,
    const char *from,
    const char *to);

std::string zlib_inflate(const std::string &input);

#endif
