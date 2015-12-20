#pragma once

#include "types.h"

namespace au {
namespace algo {

    bstr sjis_to_utf8(const bstr &input);
    bstr utf16_to_utf8(const bstr &input);

    bstr utf8_to_sjis(const bstr &input);
    bstr utf8_to_utf16(const bstr &input);

} }
