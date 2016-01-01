#pragma once

#include "base_decoder.h"

namespace au {
namespace dec {

    class BaseFileDecoder : public BaseDecoder
    {
    public:
        virtual ~BaseFileDecoder() {}

        NamingStrategy naming_strategy() const override;

        void accept(IDecoderVisitor &visitor) const override;

        std::unique_ptr<io::File> decode(
            const Logger &logger, io::File &input_file) const;

    protected:
        virtual std::unique_ptr<io::File> decode_impl(
            const Logger &logger, io::File &input_file) const = 0;
    };

} }
