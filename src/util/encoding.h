#ifndef AU_UTIL_ENCODING_H
#define AU_UTIL_ENCODING_H
#include <string>

namespace au {
namespace util {

    std::string convert_encoding(
        const std::string &input,
        const std::string &from,
        const std::string &to);

    std::string sjis_to_utf8(const std::string &input);
    std::string utf8_to_sjis(const std::string &input);

} }

#endif
