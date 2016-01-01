#include "util/file_from_image.h"
#include "enc/png/png_image_encoder.h"

using namespace au;
using namespace au::util;

std::unique_ptr<io::File> util::file_from_image(
    const res::Image &image, const io::path &path)
{
    Logger dummy_logger;
    const auto png_image_encoder = enc::png::PngImageEncoder();
    return png_image_encoder.encode(dummy_logger, image, path);
}
