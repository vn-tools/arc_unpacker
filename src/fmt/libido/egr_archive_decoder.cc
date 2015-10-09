#include "fmt/libido/egr_archive_decoder.h"
#include "err.h"
#include "util/file_from_grid.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::libido;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t width, height;
    };
}

bool EgrArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.has_extension("egr");
}

std::unique_ptr<fmt::ArchiveMeta>
    EgrArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto i = 0;
    auto meta = std::make_unique<ArchiveMeta>();
    while (!arc_file.io.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->width = arc_file.io.read_u32_le();
        entry->height = arc_file.io.read_u32_le();
        if (arc_file.io.read_u32_le() != entry->width * entry->height)
            throw err::BadDataSizeError();
        entry->offset = arc_file.io.tell();
        arc_file.io.skip(0x574 + entry->width * entry->height);
        entry->name = util::format("Image%03d.png", i++);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> EgrArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
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
        entry->width,
        entry->height,
        arc_file.io.read(entry->width * entry->height),
        palette);

    return util::file_from_grid(pixels, entry->name);
}

static auto dummy = fmt::register_fmt<EgrArchiveDecoder>("libido/egr");
