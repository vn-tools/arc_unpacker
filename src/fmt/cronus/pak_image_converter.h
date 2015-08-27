#pragma once

#include "fmt/converter.h"

namespace au {
namespace fmt {
namespace cronus {

    class PakImageConverter final : public Converter
    {
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }
