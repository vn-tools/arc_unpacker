#pragma once

#include "fmt/audio_decoder.h"

namespace au {
namespace fmt {
namespace cri {

    class HcaAudioDecoder final : public AudioDecoder
    {
    protected:
        bool is_recognized_impl(io::File &input_file) const override;
        sfx::Wave decode_impl(io::File &input_file) const override;
    };

} } }
