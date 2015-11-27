#pragma once

#include "idecoder.h"
#include "pix/image.h"

namespace au {
namespace fmt {

    class ImageDecoder : public BaseDecoder
    {
    public:
        virtual ~ImageDecoder();

        void unpack(
            io::File &input_file,
            const FileSaver &file_saver) const override;

        NamingStrategy naming_strategy() const override;

        pix::Image decode(io::File &input_file) const;

    protected:
        virtual pix::Image decode_impl(io::File &input_file) const = 0;
    };

} }
