#pragma once

#include "dec/base_file_decoder.h"

namespace au {
namespace dec {
namespace eagls {

    class PakScriptFileDecoder final : public BaseFileDecoder
    {
    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<io::File> decode_impl(
            const Logger &logger, io::File &input_file) const override;
    };

} } }
