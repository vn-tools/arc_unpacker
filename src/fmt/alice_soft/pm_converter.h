#pragma once

#include "fmt/converter.h"
#include "util/image.h"

namespace au {
namespace fmt {
namespace alice_soft {

    class PmConverter final : public Converter
    {
    public:
        std::unique_ptr<util::Image> decode_to_image(const bstr &data) const;
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }
