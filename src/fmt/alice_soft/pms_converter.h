#pragma once

#include "fmt/converter.h"
#include "util/image.h"

namespace au {
namespace fmt {
namespace alice_soft {

    class PmsConverter final : public Converter
    {
    public:
        static bstr decompress_8bit(io::IO &, size_t width, size_t height);
        static bstr decompress_16bit(io::IO &, size_t width, size_t height);
        static std::unique_ptr<util::Image> decode_to_image(const bstr &data);
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }
