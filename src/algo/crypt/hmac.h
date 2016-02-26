#pragma once

#include "types.h"

namespace au {
namespace algo {
namespace crypt {

    enum class HmacKind : u8
    {
        Sha512
    };

    bstr hmac(const bstr &input, const bstr &key, const HmacKind hmac_kind);

} } }
