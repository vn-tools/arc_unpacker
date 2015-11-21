#include "fmt/minato_soft/fil_image_decoder.h"

using namespace au;
using namespace au::fmt::minato_soft;

bool FilImageDecoder::is_recognized_impl(File &file) const
{
    const auto width = file.stream.read_u32_le();
    const auto height = file.stream.read_u32_le();
    return file.stream.tell() + width * height == file.stream.size();
}

pix::Grid FilImageDecoder::decode_impl(File &file) const
{
    const auto width = file.stream.read_u32_le();
    const auto height = file.stream.read_u32_le();
    const auto data =  file.stream.read(width * height);
    return pix::Grid(width, height, data, pix::Format::Gray8);
}

static auto dummy = fmt::register_fmt<FilImageDecoder>("minato-soft/fil");
