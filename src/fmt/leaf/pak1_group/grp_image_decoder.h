#pragma once

#include "fmt/image_decoder.h"

namespace au {
namespace fmt {
namespace leaf {

    class GrpImageDecoder final : public ImageDecoder
    {
    public:
        using ImageDecoder::decode;
        res::Image decode(
            io::File &file,
            std::shared_ptr<io::File> palette_file,
            std::shared_ptr<io::File> mask_file) const;

    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        res::Image decode_impl(io::File &input_file) const override;
    };

} } }
