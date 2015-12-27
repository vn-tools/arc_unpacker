#pragma once

#include "fmt/image_decoder.h"

namespace au {
namespace fmt {
namespace nekopack {

    class MaskedBmpImageDecoder final : public ImageDecoder
    {
    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        res::Image decode_impl(
            const Logger &logger, io::File &input_file) const override;
    };

} } }
