#include "fmt/touhou/tfbm_image_decoder.h"
#include <map>
#include "err.h"
#include "io/buffered_io.h"
#include "util/format.h"
#include "util/image.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

static const bstr pal_magic = "TFPA\x00"_b;
static const bstr magic = "TFBM\x00"_b;

namespace
{
    using PaletteMap = std::map<std::string, std::shared_ptr<pix::Palette>>;
}

struct TfbmImageDecoder::Priv final
{
    PaletteMap palette_map;
};

TfbmImageDecoder::TfbmImageDecoder() : p(new Priv)
{
}

TfbmImageDecoder::~TfbmImageDecoder()
{
}

void TfbmImageDecoder::add_palette(
    const std::string &name, const bstr &palette_data)
{
    io::BufferedIO palette_io(palette_data);
    if (palette_io.read(pal_magic.size()) != pal_magic)
        throw err::RecognitionError();

    io::BufferedIO colors_io(
        util::pack::zlib_inflate(
            palette_io.read(
                palette_io.read_u32_le())));

    p->palette_map[name] = std::shared_ptr<pix::Palette>(
        new pix::Palette(256, colors_io, pix::Format::BGRA5551));
}

bool TfbmImageDecoder::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> TfbmImageDecoder::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    auto bit_depth = file.io.read_u8();
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    auto stride = file.io.read_u32_le();
    auto source_size = file.io.read_u32_le();
    io::BufferedIO source_io(util::pack::zlib_inflate(file.io.read_to_eof()));

    std::shared_ptr<pix::Palette> palette;
    if (bit_depth == 8)
    {
        u32 palette_number = 0;
        auto path = boost::filesystem::path(file.name);
        path.remove_filename();
        path /= util::format("palette%03d.bmp", palette_number);

        auto it = p->palette_map.find(path.generic_string());
        palette = it != p->palette_map.end()
            ? it->second
            : std::shared_ptr<pix::Palette>(new pix::Palette(256));
    }

    pix::Grid pixels(width, height);
    auto *pixels_ptr = &pixels.at(0, 0);
    for (size_t y : util::range(height))
    for (size_t x : util::range(stride))
    {
        pix::Pixel pixel;

        switch (bit_depth)
        {
            case 32:
                pixel = pix::read<pix::Format::BGRA8888>(source_io);
                break;

            case 16:
                pixel = pix::read<pix::Format::BGR565>(source_io);
                break;

            case 8:
                pixel = (*palette)[source_io.read_u8()];
                break;

            default:
                throw err::UnsupportedBitDepthError(bit_depth);
        }

        if (x < width)
            *pixels_ptr++ = pixel;
    }

    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<TfbmImageDecoder>("th/tfbm");
