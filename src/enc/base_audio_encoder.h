#pragma once

#include "io/file.h"
#include "logger.h"
#include "res/audio.h"

namespace au {
namespace enc {

    class BaseAudioEncoder
    {
    public:
        virtual ~BaseAudioEncoder() {}

        std::unique_ptr<io::File> encode(
            const Logger &logger,
            const res::Audio &input_audio,
            const io::path &name) const;

    protected:
        virtual void encode_impl(
            const Logger &logger,
            const res::Audio &input_audio,
            io::File &output_file) const = 0;
    };

} }
