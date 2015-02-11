#ifndef STRING_EX_H
#define STRING_EX_H
#include <string>

std::string convert_encoding(
    const std::string &input,
    const std::string &from,
    const std::string &to);

std::string zlib_inflate(const std::string &input);

#endif
