#pragma once

#include "io/base_bit_stream.h"
#include "io/base_byte_stream.h"

namespace au {
namespace io {

    class MsbBitStream final : public BaseBitStream
    {
    public:
        MsbBitStream(const bstr &input);
        MsbBitStream(io::BaseByteStream &input_stream);
        u32 read(const size_t n) override;
    };

} }
