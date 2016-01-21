#pragma once

#include "io/base_bit_stream.h"
#include "io/base_byte_stream.h"

namespace au {
namespace io {

    class LsbBitStream final : public BaseBitStream
    {
    public:
        LsbBitStream(const bstr &input);
        LsbBitStream(io::BaseByteStream &input_stream);
        u32 read(const size_t n) override;
    };

} }
