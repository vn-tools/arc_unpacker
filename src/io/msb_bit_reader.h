#pragma once

#include "io/base_bit_reader.h"

namespace au {
namespace io {

    class MsbBitReader final : public BaseBitReader
    {
    public:
        MsbBitReader(const bstr &input);
        MsbBitReader(io::Stream &input_stream);
        u32 get(const size_t n) override;
    };

} }
