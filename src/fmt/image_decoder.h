#pragma once

#include "idecoder.h"
#include "pix/grid.h"

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

        pix::Grid decode(io::File &input_file) const;

    protected:
        virtual pix::Grid decode_impl(io::File &input_file) const = 0;
    };

} }
