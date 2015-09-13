#pragma once

#include "io/bit_reader.h"

namespace au {
namespace fmt {
namespace entis {
namespace common {

    class Decoder
    {
    public:
        Decoder();
        virtual ~Decoder();

        void set_input(const bstr &data);
        virtual void reset() = 0;
        virtual void decode(u8 *output, size_t output_size) = 0;

        std::unique_ptr<io::BitReader> bit_reader;
    };

} } } }
