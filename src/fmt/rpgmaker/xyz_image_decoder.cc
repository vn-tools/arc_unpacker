#include "fmt/rpgmaker/xyz_image_decoder.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::fmt::rpgmaker;

static const bstr magic = "XYZ1"_b;

bool XyzImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image XyzImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    u16 width = input_file.stream.read_u16_le();
    u16 height = input_file.stream.read_u16_le();

    bstr data = algo::pack::zlib_inflate(input_file.stream.read_to_eof());

    io::MemoryStream data_stream(data);
    auto pal_data = data_stream.read(256 * 3);
    auto pix_data = data_stream.read_to_eof();

    res::Palette palette(256, pal_data, res::PixelFormat::RGB888);
    return res::Image(width, height, pix_data, palette);
}

static auto dummy = fmt::register_fmt<XyzImageDecoder>("rpgmaker/xyz");
