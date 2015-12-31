#pragma once

#include "io/bit_reader.h"

namespace au {
namespace fmt {
namespace active_soft {

    class CustomBitReader final : public io::BaseBitReader
    {
    public:
        CustomBitReader(const bstr &input);
        u32 get(const size_t bits) override;
    };

} } }
