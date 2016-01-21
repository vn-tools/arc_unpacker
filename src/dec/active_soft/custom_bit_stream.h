#pragma once

#include "io/base_bit_stream.h"

namespace au {
namespace dec {
namespace active_soft {

    class CustomBitStream final : public io::BaseBitStream
    {
    public:
        CustomBitStream(const bstr &input);
        u32 read(const size_t bits) override;
    };

} } }
