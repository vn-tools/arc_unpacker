// EGR image archive
//
// Company:   Libido
// Engine:    -
// Extension: .egr
//
// Known games:
// - Libido 7

#include "fmt/libido/egr_archive.h"
#include "util/format.h"
#include "util/image.h"
#include "util/range.h"
#include "util/require.h"

using namespace au;
using namespace au::fmt::libido;

namespace
{
    struct TableEntry
    {
        std::string name;
        size_t size;
        size_t offset;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

bool EgrArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("egr");
}

void EgrArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto i = 0;
    while (!arc_file.io.eof())
    {
        auto width = arc_file.io.read_u32_le();
        auto height = arc_file.io.read_u32_le();
        util::require(arc_file.io.read_u32_le() == width * height);

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
        file_saver.save(util::Image::from_pixels(pixels)->create_file(name));
    }
}

static auto dummy = fmt::Registry::add<EgrArchive>("libido/egr");
