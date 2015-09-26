#include "fmt/rpgmaker/xyz_image_decoder.h"
#include "io/buffered_io.h"
#include "util/image.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::rpgmaker;

static const bstr magic = "XYZ1"_b;

bool XyzImageDecoder::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> XyzImageDecoder::decode_internal(File &file) const
{
    file.io.skip(magic.size());

    u16 width = file.io.read_u16_le();
    u16 height = file.io.read_u16_le();

    bstr data = util::pack::zlib_inflate(file.io.read_to_eof());

    io::BufferedIO data_io(data);
    auto pal_data = data_io.read(256 * 3);
    auto pix_data = data_io.read_to_eof();

    pix::Palette palette(256, pal_data, pix::Format::RGB888);
    pix::Grid pixels(width, height, pix_data, palette);
    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<XyzImageDecoder>("rm/xyz");
