#pragma once

#include "fmt/file_decoder.h"

namespace au {
namespace fmt {
namespace amuse_craft {

    class BgmAudioDecoder final : public FileDecoder
    {
    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<io::File> decode_impl(
            const Logger &logger, io::File &input_file) const override;
    };

} } }
