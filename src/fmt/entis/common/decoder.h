#pragma once

#include "io/bit_reader.h"

namespace au {
namespace fmt {
namespace entis {
namespace common {

    class Decoder
    {
    public:
        Decoder(const bstr &data);
        virtual ~Decoder();
        virtual void decode(u8 *output, size_t output_size) = 0;

        io::BitReader bit_reader;
    };

} } } }
