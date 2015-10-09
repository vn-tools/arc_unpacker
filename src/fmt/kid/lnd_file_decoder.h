#pragma once

#include "fmt/file_decoder.h"

namespace au {
namespace fmt {
namespace kid {

    class LndFileDecoder final : public FileDecoder
    {
    public:
        static bstr decompress_raw_data(const bstr &input, size_t size_orig);
    protected:
        bool is_recognized_impl(File &) const override;
        std::unique_ptr<File> decode_impl(File &) const override;
    };

} } }
