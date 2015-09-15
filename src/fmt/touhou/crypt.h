#pragma once

#include "types.h"

namespace au {
namespace fmt {
namespace touhou {

    struct DecryptorContext final
    {
        bool operator ==(const DecryptorContext &other) const;

        u8 key;
        u8 step;
        size_t block_size;
        size_t limit;
    };

    bstr decrypt(const bstr &input, const DecryptorContext &context);

} } }
