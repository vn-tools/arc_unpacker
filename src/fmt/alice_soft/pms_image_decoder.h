#pragma once

#include "fmt/base_image_decoder.h"

namespace au {
namespace fmt {
namespace alice_soft {

    class PmsImageDecoder final : public BaseImageDecoder
    {
    public:
        static bstr decompress_8bit(
            io::Stream &input_stream, const size_t width, const size_t height);

        static bstr decompress_16bit(
            io::Stream &input_stream, const size_t width, const size_t height);

    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        res::Image decode_impl(
            const Logger &logger, io::File &input_file) const override;
    };

} } }
