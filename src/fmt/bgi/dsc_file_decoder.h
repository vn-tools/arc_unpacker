#pragma once

#include "fmt/base_file_decoder.h"

namespace au {
namespace fmt {
namespace bgi {

    class DscFileDecoder final : public BaseFileDecoder
    {
    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<io::File> decode_impl(
            const Logger &logger, io::File &input_file) const override;
    };

} } }
