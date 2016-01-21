#pragma once

#include <memory>
#include "io/base_bit_stream.h"

namespace au {
namespace dec {
namespace entis {
namespace common {

    class BaseDecoder
    {
    public:
        BaseDecoder() {}
        virtual ~BaseDecoder() {}

        virtual void set_input(const bstr &data);
        virtual void reset() = 0;
        virtual void decode(u8 *ouptut, const size_t output_size) = 0;

        std::unique_ptr<io::BaseBitStream> bit_stream;
    };

} } } }
