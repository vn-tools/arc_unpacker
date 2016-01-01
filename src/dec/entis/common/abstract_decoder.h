#pragma once

#include <memory>
#include "io/ibit_reader.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    class AbstractDecoder
    {
    public:
        AbstractDecoder() {}
        virtual ~AbstractDecoder() {}

        virtual void set_input(const bstr &data);
        virtual void reset() = 0;
        virtual void decode(u8 *ouptut, size_t output_size) = 0;

        std::unique_ptr<io::IBitReader> bit_reader;
    };

} } } }
