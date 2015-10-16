#pragma once

#include "fmt/image_decoder.h"

namespace au {
namespace fmt {
namespace amuse_craft {

    class PgdC00ImageDecoder final : public ImageDecoder
    {
    protected:
        bool is_recognized_impl(File &) const override;
        pix::Grid decode_impl(File &) const override;
    };

} } }
