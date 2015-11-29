#include "fmt/twilight_frontier/tfbm_image_decoder.h"
#include <map>
#include "err.h"
#include "io/memory_stream.h"
#include "util/format.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

static const bstr pal_magic = "TFPA\x00"_b;
static const bstr magic = "TFBM\x00"_b;

namespace
{
    using PaletteMap = std::map<io::path, std::shared_ptr<res::Palette>>;
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

void TfbmImageDecoder::clear_palettes()
{
    p->palette_map.clear();
}

void TfbmImageDecoder::add_palette(
    const io::path &path, const bstr &palette_data)
{
    io::MemoryStream palette_stream(palette_data);
    if (palette_stream.read(pal_magic.size()) != pal_magic)
        throw err::RecognitionError();

    io::MemoryStream colors_stream(
        util::pack::zlib_inflate(
            palette_stream.read(
                palette_stream.read_u32_le())));

    p->palette_map[path] = std::make_shared<res::Palette>(
        256, colors_stream, res::PixelFormat::BGRA5551);
}

bool TfbmImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image TfbmImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    const auto bit_depth = input_file.stream.read_u8();
    const auto width = input_file.stream.read_u32_le();
    const auto height = input_file.stream.read_u32_le();
    const auto stride = input_file.stream.read_u32_le();
    const auto source_size = input_file.stream.read_u32_le();
    io::MemoryStream source_stream(
        util::pack::zlib_inflate(input_file.stream.read_to_eof()));

    std::shared_ptr<res::Palette> palette;
    if (bit_depth == 8)
    {
        u32 palette_number = 0;
        const auto path = input_file.path.parent()
            / util::format("palette%03d.bmp", palette_number);

        auto it = p->palette_map.find(path);
        palette = it != p->palette_map.end()
            ? it->second
            : std::make_shared<res::Palette>(256);
    }

    res::Image image(width, height);
    auto *pixels_ptr = &image.at(0, 0);
    for (const size_t y : util::range(height))
    for (const size_t x : util::range(stride))
    {
        res::Pixel pixel;

        switch (bit_depth)
        {
            case 32:
                pixel = res::read_pixel<res::PixelFormat::BGRA8888>(
                    source_stream);
                break;

            case 16:
                pixel = res::read_pixel<res::PixelFormat::BGR565>(
                    source_stream);
                break;

            case 8:
                pixel = (*palette)[source_stream.read_u8()];
                break;

            default:
                throw err::UnsupportedBitDepthError(bit_depth);
        }

        if (x < width)
            *pixels_ptr++ = pixel;
    }

    return image;
}

static auto dummy
    = fmt::register_fmt<TfbmImageDecoder>("twilight-frontier/tfbm");
