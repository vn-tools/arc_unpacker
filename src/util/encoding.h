#ifndef AU_UTIL_ENCODING_H
#define AU_UTIL_ENCODING_H
#include "types.h"

namespace au {
namespace util {

    bstr convert_encoding(
        const bstr &input,
        const std::string &from,
        const std::string &to);

    bstr sjis_to_utf8(const bstr &input);
    bstr utf8_to_sjis(const bstr &input);

} }

#endif
