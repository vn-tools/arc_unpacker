#pragma once

#include "fmt/file_decoder.h"

namespace au {
namespace fmt {
namespace eagls {

    class GrImageDecoder final : public FileDecoder
    {
    protected:
        bool is_recognized_internal(File &) const override;
        std::unique_ptr<File> decode_internal(File &) const override;
    };

} } }
