#include "fmt/leaf/leafpack_group/lf3_image_decoder.h"
#include "fmt/leaf/common/custom_lzss.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LEAF64K\x00"_b;

bool Lf3ImageDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

pix::Grid Lf3ImageDecoder::decode_impl(File &file) const
{
    file.stream.seek(magic.size());
    file.stream.skip(4);
    const auto width = file.stream.read_u16_le();
    const auto height = file.stream.read_u16_le();

    const auto data_pos = file.stream.read_u32_le();

    file.stream.seek(data_pos);
    const auto data = common::custom_lzss_decompress(
        file.stream.read_to_eof(), width * height * 2);
    pix::Grid image(width, height, data, pix::Format::BGR555X);
    image.flip_vertically();
    return image;
}

static auto dummy = fmt::register_fmt<Lf3ImageDecoder>("leaf/lf3");
