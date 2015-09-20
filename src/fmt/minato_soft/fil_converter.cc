#include "fmt/minato_soft/fil_converter.h"
#include "util/image.h"

using namespace au;
using namespace au::fmt::minato_soft;

bool FilConverter::is_recognized_internal(File &file) const
{
    return file.io.read_u32_le() * file.io.read_u32_le() + 8 == file.io.size();
}

std::unique_ptr<File> FilConverter::decode_internal(File &file) const
{
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    auto data =  file.io.read(width * height);
    pix::Grid pixels(width, height, data, pix::Format::Gray8);
    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<FilConverter>("minato/fil");
