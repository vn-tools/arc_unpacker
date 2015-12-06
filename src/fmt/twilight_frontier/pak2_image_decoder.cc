#include "fmt/twilight_frontier/pak2_image_decoder.h"
#include <map>
#include "algo/format.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

namespace
{
    using PaletteMap = std::map<io::path, std::shared_ptr<res::Palette>>;
}

struct Pak2ImageDecoder::Priv final
{
    PaletteMap palette_map;
};

Pak2ImageDecoder::Pak2ImageDecoder() : p(new Priv)
{
}

Pak2ImageDecoder::~Pak2ImageDecoder()
{
}

void Pak2ImageDecoder::clear_palettes()
{
    p->palette_map.clear();
}

void Pak2ImageDecoder::add_palette(
    const io::path &path, const bstr &palette_data)
{
    io::MemoryStream palette_stream(palette_data);
    palette_stream.skip(1);
    p->palette_map[path] = std::make_shared<res::Palette>(
        256, palette_stream, res::PixelFormat::BGRA5551);
}

bool Pak2ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("cv2");
}

res::Image Pak2ImageDecoder::decode_impl(io::File &input_file) const
{
    const auto bit_depth = input_file.stream.read_u8();
    const auto width = input_file.stream.read_u32_le();
    const auto height = input_file.stream.read_u32_le();
    const auto stride = input_file.stream.read_u32_le();
    const auto palette_number = input_file.stream.read_u32_le();
    io::MemoryStream source_stream(input_file.stream);

    std::shared_ptr<res::Palette> palette;
    if (bit_depth == 8)
    {
        const auto path = input_file.path.parent()
            / algo::format("palette%03d.pal", palette_number);

        auto it = p->palette_map.find(path);
        palette = it != p->palette_map.end()
            ? it->second
            : std::make_shared<res::Palette>(256);
    }

    res::Image image(width, height);
    for (const size_t y : algo::range(height))
    for (const size_t x : algo::range(stride))
    {
        res::Pixel pixel;

        switch (bit_depth)
        {
            case 32:
            case 24:
                pixel = res::read_pixel<res::PixelFormat::BGRA8888>(
                    source_stream);
                break;

            case 8:
                pixel = (*palette)[source_stream.read_u8()];
                break;

            default:
                throw err::UnsupportedBitDepthError(bit_depth);
        }

        if (x < width)
            image.at(x, y) = pixel;
    }

    return image;
}

static auto dummy
    = fmt::register_fmt<Pak2ImageDecoder>("twilight-frontier/pak2-gfx");
