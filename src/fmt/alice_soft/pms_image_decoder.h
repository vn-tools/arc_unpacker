#pragma once

#include "fmt/image_decoder.h"

namespace au {
namespace fmt {
namespace alice_soft {

    class PmsImageDecoder final : public ImageDecoder
    {
    public:
        static bstr decompress_8bit(io::IO &, size_t width, size_t height);
        static bstr decompress_16bit(io::IO &, size_t width, size_t height);
    protected:
        bool is_recognized_internal(File &) const override;
        pix::Grid decode_internal(File &) const override;
    };

} } }
