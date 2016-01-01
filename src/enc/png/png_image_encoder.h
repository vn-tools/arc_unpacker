#pragma once

#include "enc/base_image_encoder.h"

namespace au {
namespace enc {
namespace png {

    class PngImageEncoder final : public BaseImageEncoder
    {
    protected:
        void encode_impl(
            const Logger &logger,
            const res::Image &input_image,
            io::File &output_file) const override;
    };

} } }
