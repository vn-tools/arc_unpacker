#pragma once

#include "io/base_bit_reader.h"

namespace au {
namespace dec {
namespace active_soft {

    class CustomBitReader final : public io::BaseBitReader
    {
    public:
        CustomBitReader(const bstr &input);
        u32 get(const size_t bits) override;
    };

} } }
