#pragma once

#include "fmt/file_decoder.h"
#include "sfx/wave.h"

namespace au {
namespace fmt {
namespace microsoft {

    class WavAudioDecoder final : public FileDecoder
    {
    public:
        sfx::Wave decode_to_wave(io::File &input_file) const;

    protected:
        bool is_recognized_impl(io::File &input_file) const override;

        std::unique_ptr<io::File> decode_impl(
            io::File &input_file) const override;
    };

} } }
