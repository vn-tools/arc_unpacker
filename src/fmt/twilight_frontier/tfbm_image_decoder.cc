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

void TfbmImageDecoder::clear_palettes()
{
    p->palette_map.clear();
}

void TfbmImageDecoder::add_palette(
    const std::string &name, const bstr &palette_data)
{
    io::MemoryStream palette_stream(palette_data);
    if (palette_stream.read(pal_magic.size()) != pal_magic)
        throw err::RecognitionError();

    io::MemoryStream colors_stream(
        util::pack::zlib_inflate(
            palette_stream.read(
                palette_stream.read_u32_le())));

    p->palette_map[name] = std::make_shared<pix::Palette>(
        256, colors_stream, pix::Format::BGRA5551);
}

bool TfbmImageDecoder::is_recognized_impl(File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

pix::Grid TfbmImageDecoder::decode_impl(File &input_file) const
{
    input_file.stream.skip(magic.size());
    auto bit_depth = input_file.stream.read_u8();
    auto width = input_file.stream.read_u32_le();
    auto height = input_file.stream.read_u32_le();
    auto stride = input_file.stream.read_u32_le();
    auto source_size = input_file.stream.read_u32_le();
    io::MemoryStream source_stream(
        util::pack::zlib_inflate(input_file.stream.read_to_eof()));

    std::shared_ptr<pix::Palette> palette;
    if (bit_depth == 8)
    {
        u32 palette_number = 0;
        auto path = boost::filesystem::path(input_file.name);
        path.remove_filename();
        path /= util::format("palette%03d.bmp", palette_number);

        auto it = p->palette_map.find(path.generic_string());
        palette = it != p->palette_map.end()
            ? it->second
            : std::make_shared<pix::Palette>(256);
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
                pixel = pix::read<pix::Format::BGRA8888>(source_stream);
                break;

            case 16:
                pixel = pix::read<pix::Format::BGR565>(source_stream);
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

    return pixels;
}

static auto dummy
    = fmt::register_fmt<TfbmImageDecoder>("twilight-frontier/tfbm");
