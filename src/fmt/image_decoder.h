#pragma once

#include "idecoder.h"
#include "res/image.h"

namespace au {
namespace fmt {

    class ImageDecoder : public BaseDecoder
    {
    public:
        virtual ~ImageDecoder() { }

        NamingStrategy naming_strategy() const override;

        void accept(IDecoderVisitor &visitor) const;

        res::Image decode(
            const Logger &logger, io::File &input_file) const;

    protected:
        virtual res::Image decode_impl(
            const Logger &logger, io::File &input_file) const = 0;
    };

} }
