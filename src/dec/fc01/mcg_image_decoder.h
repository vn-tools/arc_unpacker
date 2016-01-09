#pragma once

#include <boost/optional.hpp>
#include "dec/base_image_decoder.h"

namespace au {
namespace dec {
namespace fc01 {

    class McgImageDecoder final : public BaseImageDecoder
    {
    public:
        McgImageDecoder();

    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        res::Image decode_impl(
            const Logger &logger, io::File &input_file) const override;

    public:
        boost::optional<u8> key;
    };

} } }
