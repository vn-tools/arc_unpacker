#include "fmt/rpgmaker/xyz_image_decoder.h"
#include "io/memory_stream.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::rpgmaker;

static const bstr magic = "XYZ1"_b;

bool XyzImageDecoder::is_recognized_impl(File &file) const
{
    return file.stream.read(magic.size()) == magic;
}

pix::Grid XyzImageDecoder::decode_impl(File &file) const
{
    file.stream.skip(magic.size());

    u16 width = file.stream.read_u16_le();
    u16 height = file.stream.read_u16_le();

    bstr data = util::pack::zlib_inflate(file.stream.read_to_eof());

    io::MemoryStream data_stream(data);
    auto pal_data = data_stream.read(256 * 3);
    auto pix_data = data_stream.read_to_eof();

    pix::Palette palette(256, pal_data, pix::Format::RGB888);
    return pix::Grid(width, height, pix_data, palette);
}

static auto dummy = fmt::register_fmt<XyzImageDecoder>("rpgmaker/xyz");
