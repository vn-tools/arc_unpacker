#pragma once

#include "io/file.h"
#include "logger.h"
#include "res/image.h"

namespace au {
namespace enc {

    class BaseImageEncoder
    {
    public:
        virtual ~BaseImageEncoder() {}

        std::unique_ptr<io::File> encode(
            const Logger &logger,
            const res::Image &input_image,
            const io::path &name) const;

    protected:
        virtual void encode_impl(
            const Logger &logger,
            const res::Image &input_image,
            io::File &output_file) const = 0;
    };

} }
