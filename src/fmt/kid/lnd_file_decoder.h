#pragma once

#include "fmt/base_file_decoder.h"

namespace au {
namespace fmt {
namespace kid {

    class LndFileDecoder final : public BaseFileDecoder
    {
    public:
        static bstr decompress_raw_data(const bstr &input, size_t size_orig);

    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<io::File> decode_impl(
            const Logger &logger, io::File &input_file) const override;
    };

} } }
