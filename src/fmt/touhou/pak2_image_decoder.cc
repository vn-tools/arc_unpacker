#include "fmt/touhou/pak2_image_decoder.h"
#include <boost/filesystem.hpp>
#include <map>
#include "err.h"
#include "io/buffered_io.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

namespace
{
    using PaletteMap = std::map<std::string, std::shared_ptr<pix::Palette>>;
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
    io::BufferedIO palette_io(palette_data);
    palette_io.skip(1);
    p->palette_map[name] = std::make_shared<pix::Palette>(
        256, palette_io, pix::Format::BGRA5551);
}

bool Pak2ImageDecoder::is_recognized_impl(File &file) const
{
    return file.has_extension("cv2");
}

pix::Grid Pak2ImageDecoder::decode_impl(File &file) const
{
    auto bit_depth = file.io.read_u8();
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    auto stride = file.io.read_u32_le();
    auto palette_number = file.io.read_u32_le();
    io::BufferedIO source_io(file.io);

    std::shared_ptr<pix::Palette> palette;
    if (bit_depth == 8)
    {
        auto path = boost::filesystem::path(file.name);
        path.remove_filename();
        path /= util::format("palette%03d.pal", palette_number);

        auto it = p->palette_map.find(path.generic_string());
        palette = it != p->palette_map.end()
            ? it->second
            : std::make_shared<pix::Palette>(256);
    }

    pix::Grid pixels(width, height);
    for (size_t y : util::range(height))
    for (size_t x : util::range(stride))
    {
        pix::Pixel pixel;

        switch (bit_depth)
        {
            case 32:
            case 24:
                pixel = pix::read<pix::Format::BGRA8888>(source_io);
                break;

            case 8:
                pixel = (*palette)[source_io.read_u8()];
                break;

            default:
                throw err::UnsupportedBitDepthError(bit_depth);
        }

        if (x < width)
            pixels.at(x, y) = pixel;
    }

    return pixels;
}

static auto dummy = fmt::Registry::add<Pak2ImageDecoder>("th/pak2-gfx");
