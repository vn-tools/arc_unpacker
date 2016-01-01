#include "enc/base_image_encoder.h"

using namespace au;
using namespace au::enc;

std::unique_ptr<io::File> BaseImageEncoder::encode(
    const Logger &logger,
    const res::Image &input_image,
    const io::path &name) const
{
    auto output_file = std::make_unique<io::File>(name, ""_b);
    encode_impl(logger, input_image, *output_file);
    return output_file;
}
