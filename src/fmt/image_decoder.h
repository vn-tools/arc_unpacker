#pragma once

#include "idecoder.h"
#include "res/image.h"

namespace au {
namespace fmt {

    class ImageDecoder : public BaseDecoder
    {
    public:
        virtual ~ImageDecoder() { }

        void unpack(
            const Logger &logger,
            io::File &input_file,
            const FileSaver &file_saver) const override;

        NamingStrategy naming_strategy() const override;

        res::Image decode(
            const Logger &logger, io::File &input_file) const;

    protected:
        virtual res::Image decode_impl(
            const Logger &logger, io::File &input_file) const = 0;
    };

} }
