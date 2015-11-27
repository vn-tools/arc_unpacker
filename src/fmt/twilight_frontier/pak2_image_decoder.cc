#include "fmt/twilight_frontier/pak2_image_decoder.h"
#include <boost/filesystem.hpp>
#include <map>
#include "err.h"
#include "io/memory_stream.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

namespace
{
    using PaletteMap = std::map<io::path, std::shared_ptr<pix::Palette>>;
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
    const std::string &name, const bstr &palette_data)
{
    io::MemoryStream palette_stream(palette_data);
    palette_stream.skip(1);
    p->palette_map[name] = std::make_shared<pix::Palette>(
        256, palette_stream, pix::Format::BGRA5551);
}

bool Pak2ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.has_extension("cv2");
}

pix::Image Pak2ImageDecoder::decode_impl(io::File &input_file) const
{
    auto bit_depth = input_file.stream.read_u8();
    auto width = input_file.stream.read_u32_le();
    auto height = input_file.stream.read_u32_le();
    auto stride = input_file.stream.read_u32_le();
    auto palette_number = input_file.stream.read_u32_le();
    io::MemoryStream source_stream(input_file.stream);

    std::shared_ptr<pix::Palette> palette;
    if (bit_depth == 8)
    {
        auto path = io::path(input_file.name).parent();
        path /= util::format("palette%03d.pal", palette_number);

        auto it = p->palette_map.find(path);
        palette = it != p->palette_map.end()
            ? it->second
            : std::make_shared<pix::Palette>(256);
    }

    pix::Image image(width, height);
    for (size_t y : util::range(height))
    for (size_t x : util::range(stride))
    {
        pix::Pixel pixel;

        switch (bit_depth)
        {
            case 32:
            case 24:
                pixel = pix::read<pix::Format::BGRA8888>(source_stream);
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
