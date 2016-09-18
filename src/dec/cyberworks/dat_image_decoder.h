#pragma once

#include "dec/base_image_decoder.h"
#include "dec/cyberworks/dat_plugin.h"

namespace au {
namespace dec {
namespace cyberworks {

    class DatImageDecoder final : public BaseImageDecoder
    {
    public:
        DatImageDecoder(const DatPlugin &plugin);

    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        res::Image decode_impl(
            const Logger &logger, io::File &input_file) const override;

    private:
        const DatPlugin &plugin;
    };

} } }
