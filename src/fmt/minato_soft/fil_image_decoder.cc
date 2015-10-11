#include "fmt/minato_soft/fil_image_decoder.h"

using namespace au;
using namespace au::fmt::minato_soft;

bool FilImageDecoder::is_recognized_impl(File &file) const
{
    return file.io.read_u32_le() * file.io.read_u32_le() + 8 == file.io.size();
}

pix::Grid FilImageDecoder::decode_impl(File &file) const
{
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    auto data =  file.io.read(width * height);
    return pix::Grid(width, height, data, pix::Format::Gray8);
}

static auto dummy = fmt::register_fmt<FilImageDecoder>("minato-soft/fil");
