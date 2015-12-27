#pragma once

#include "fmt/audio_decoder.h"

namespace au {
namespace fmt {
namespace real_live {

    class NwaAudioDecoder final : public AudioDecoder
    {
    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        res::Audio decode_impl(
            const Logger &logger, io::File &input_file) const override;
    };

} } }
