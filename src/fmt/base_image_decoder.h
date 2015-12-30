#pragma once

#include "base_decoder.h"
#include "res/image.h"

namespace au {
namespace fmt {

    class BaseImageDecoder : public BaseDecoder
    {
    public:
        virtual ~BaseImageDecoder() { }

        NamingStrategy naming_strategy() const override;

        void accept(IDecoderVisitor &visitor) const override;

        res::Image decode(
            const Logger &logger, io::File &input_file) const;

    protected:
        virtual res::Image decode_impl(
            const Logger &logger, io::File &input_file) const = 0;
    };

} }
