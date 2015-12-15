#pragma once

#include "types.h"

namespace au {
namespace algo {

    bstr convert_locale(
        const bstr &input,
        const std::string &from,
        const std::string &to);

    bstr sjis_to_utf8(const bstr &input);
    bstr utf16_to_utf8(const bstr &input);

    bstr utf8_to_sjis(const bstr &input);
    bstr utf8_to_utf16(const bstr &input);

} }
