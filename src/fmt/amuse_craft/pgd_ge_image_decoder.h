#pragma once

#include "fmt/image_decoder.h"

namespace au {
namespace fmt {
namespace amuse_craft {

    class PgdGeImageDecoder final : public ImageDecoder
    {
    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        pix::Image decode_impl(io::File &input_file) const override;
    };

} } }
