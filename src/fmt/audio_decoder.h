#pragma once

#include "idecoder.h"
#include "sfx/audio.h"

namespace au {
namespace fmt {

    class AudioDecoder : public BaseDecoder
    {
    public:
        virtual ~AudioDecoder();

        void unpack(
            io::File &input_file,
            const FileSaver &file_saver) const override;

        NamingStrategy naming_strategy() const override;

        sfx::Audio decode(io::File &input_file) const;

    protected:
        virtual sfx::Audio decode_impl(io::File &input_file) const = 0;
    };

} }
