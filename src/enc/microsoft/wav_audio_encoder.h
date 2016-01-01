#pragma once

#include "enc/base_audio_encoder.h"

namespace au {
namespace enc {
namespace microsoft {

    class WavAudioEncoder final : public BaseAudioEncoder
    {
    protected:
        void encode_impl(
            const Logger &logger,
            const res::Audio &input_audio,
            io::File &output_file) const override;
    };

} } }
