#pragma once

#include "fmt/audio_decoder.h"

namespace au {
namespace fmt {
namespace twilight_frontier {

    class TfwaAudioDecoder final : public AudioDecoder
    {
    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        res::Audio decode_impl(
            const Logger &logger, io::File &input_file) const override;
    };

} } }
