// PAK2 image file
//
// Company:   Team Shanghai Alice
// Engine:    -
// Extension: .cv2
//
// Known games:
// - [Team Shanghai Alice] [080525] TH10.5 - Scarlet Weather Rhapsody
// - [Team Shanghai Alice] [090815] TH12.3 - Unthinkable Natural Law

#include <boost/filesystem.hpp>
#include <map>
#include "fmt/touhou/pak2_image_converter.h"
#include "io/buffered_io.h"
#include "util/format.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

namespace
{
    using PaletteMap = std::map<std::string, std::shared_ptr<pix::Palette>>;
}

struct Pak2ImageConverter::Priv
{
    PaletteMap palette_map;
};

Pak2ImageConverter::Pak2ImageConverter() : p(new Priv)
{
}

Pak2ImageConverter::~Pak2ImageConverter()
{
}

void Pak2ImageConverter::add_palette(
    const std::string &name, const bstr &palette_data)
{
    io::BufferedIO palette_io(palette_data);
    palette_io.skip(1);
    p->palette_map[name] = std::shared_ptr<pix::Palette>(
        new pix::Palette(256, palette_io, pix::Format::BGRA5551));
}

bool Pak2ImageConverter::is_recognized_internal(File &file) const
{
    return file.has_extension("cv2");
}

std::unique_ptr<File> Pak2ImageConverter::decode_internal(File &file) const
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
            : std::shared_ptr<pix::Palette>(new pix::Palette(256));
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
                throw std::runtime_error("Unsupported channel count");
        }

        if (x < width)
            pixels.at(x, y) = pixel;
    }

    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<Pak2ImageConverter>("th/pak2-gfx");
