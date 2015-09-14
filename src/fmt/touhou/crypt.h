#pragma once

#include "types.h"

namespace au {
namespace fmt {
namespace touhou {

    struct DecryptorContext final
    {
        u8 key;
        u8 step;
        size_t block_size;
        size_t limit;

        bool operator ==(const DecryptorContext &other) const;
    };

    bstr decrypt(const bstr &input, const DecryptorContext &context);

} } }
