#include "fmt/touhou/pak1_image_archive_decoder.h"
#include "err.h"
#include "io/buffered_io.h"
#include "util/format.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::touhou;

static std::unique_ptr<File> read_image(
    io::IO &arc_io, size_t index, const pix::Palette &palette)
{
    auto width = arc_io.read_u32_le();
    auto height = arc_io.read_u32_le();
    arc_io.skip(4);
    auto bit_depth = arc_io.read_u8();
    size_t source_size = arc_io.read_u32_le();
    io::BufferedIO source_io(arc_io, source_size);

    pix::Grid pixels(width, height);
    auto *pixels_ptr = &pixels.at(0, 0);
    while (source_io.tell() < source_io.size())
    {
        size_t repeat;
        pix::Pixel pixel;

        switch (bit_depth)
        {
            case 32:
                repeat = source_io.read_u32_le();
                pixel = pix::read<pix::Format::BGRA8888>(source_io);
                break;

            case 24:
                repeat = source_io.read_u32_le();
                pixel = pix::read<pix::Format::BGR888X>(source_io);
                break;

            case 16:
                repeat = source_io.read_u16_le();
                pixel = pix::read<pix::Format::BGRA5551>(source_io);
                break;

            case 8:
                repeat = source_io.read_u8();
                pixel = palette[source_io.read_u8()];
                break;

            default:
                throw err::UnsupportedBitDepthError(bit_depth);
        }

        while (repeat--)
            *pixels_ptr++ = pixel;
    }

    auto name = util::format("%04d", index);
    return util::Image::from_pixels(pixels)->create_file(name);
}

bool Pak1ImageArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    if (!arc_file.has_extension("dat"))
        return false;
    auto palette_count = arc_file.io.read_u8();
    arc_file.io.skip(palette_count * 512);
    while (!arc_file.io.eof())
    {
        arc_file.io.skip(4 * 3);
        arc_file.io.skip(1);
        arc_file.io.skip(arc_file.io.read_u32_le());
    }
    return arc_file.io.eof();
}

void Pak1ImageArchiveDecoder::unpack_internal(
    File &arc_file, FileSaver &saver) const
{
    auto palette_count = arc_file.io.read_u8();
    std::vector<std::unique_ptr<pix::Palette>> palettes;
    for (auto p : util::range(palette_count))
    {
        palettes.push_back(
            std::unique_ptr<pix::Palette>(new pix::Palette(
                256, arc_file.io.read(512), pix::Format::BGRA5551)));
    }
    palettes.push_back(std::unique_ptr<pix::Palette>(new pix::Palette(256)));

    size_t i = 0;
    while (arc_file.io.tell() < arc_file.io.size())
        saver.save(read_image(arc_file.io, i++, *palettes[0]));
}

static auto dummy = fmt::Registry::add<Pak1ImageArchiveDecoder>("th/pak1-gfx");
