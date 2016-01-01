#pragma once

#include "io/base_bit_reader.h"

namespace au {
namespace io {

    class LsbBitReader final : public BaseBitReader
    {
    public:
        LsbBitReader(const bstr &input);
        LsbBitReader(io::IStream &input_stream);
        u32 get(const size_t n) override;
    };

} }
