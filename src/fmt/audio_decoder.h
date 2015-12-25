#pragma once

#include "idecoder.h"
#include "res/audio.h"

namespace au {
namespace fmt {

    class AudioDecoder : public BaseDecoder
    {
    public:
        virtual ~AudioDecoder() { }

        void unpack(
            const Logger &logger,
            io::File &input_file,
            const FileSaver &file_saver) const override;

        NamingStrategy naming_strategy() const override;

        res::Audio decode(const Logger &logger, io::File &input_file) const;

    protected:
        virtual res::Audio decode_impl(
            const Logger &logger, io::File &input_file) const = 0;
    };

} }
