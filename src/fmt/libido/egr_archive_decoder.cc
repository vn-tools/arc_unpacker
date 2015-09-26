#include "fmt/libido/egr_archive_decoder.h"
#include "err.h"
#include "util/format.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::libido;

namespace
{
    struct TableEntry final
    {
        std::string name;
        size_t size;
        size_t offset;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

bool EgrArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("egr");
}

void EgrArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    auto i = 0;
    while (!arc_file.io.eof())
    {
        auto width = arc_file.io.read_u32_le();
        auto height = arc_file.io.read_u32_le();
        if (arc_file.io.read_u32_le() != width * height)
            throw err::BadDataSizeError();

        pix::Palette palette(256);
        for (auto i : util::range(palette.size()))
        {
            arc_file.io.skip(1);
            palette[i].a = 0xFF;
            palette[i].b = arc_file.io.read_u8();
            palette[i].r = arc_file.io.read_u8();
            palette[i].g = arc_file.io.read_u8();
        }

        arc_file.io.skip(0x174);
        pix::Grid pixels(
            width, height, arc_file.io.read(width * height), palette);

        auto name = util::format("Image%03d.png", i++);
        saver.save(util::Image::from_pixels(pixels)->create_file(name));
    }
}

static auto dummy = fmt::Registry::add<EgrArchiveDecoder>("libido/egr");
