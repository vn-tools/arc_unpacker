#pragma once

#include "fmt/converter.h"

namespace au {
namespace fmt {
namespace microsoft {

    class DdsConverter final : public Converter
    {
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }
