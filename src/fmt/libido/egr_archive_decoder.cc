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

bool EgrArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.has_extension("egr");
}

std::unique_ptr<fmt::ArchiveMeta>
    EgrArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    auto i = 0;
    auto meta = std::make_unique<ArchiveMeta>();
    while (!input_file.stream.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->width = input_file.stream.read_u32_le();
        entry->height = input_file.stream.read_u32_le();
        if (input_file.stream.read_u32_le() != entry->width * entry->height)
            throw err::BadDataSizeError();
        entry->offset = input_file.stream.tell();
        input_file.stream.skip(0x574 + entry->width * entry->height);
        entry->name = util::format("Image%03d.png", i++);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> EgrArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    pix::Palette palette(256);
    for (auto i : util::range(palette.size()))
    {
        input_file.stream.skip(1);
        palette[i].a = 0xFF;
        palette[i].b = input_file.stream.read_u8();
        palette[i].r = input_file.stream.read_u8();
        palette[i].g = input_file.stream.read_u8();
    }

    input_file.stream.skip(0x174);
    pix::Grid pixels(
        entry->width,
        entry->height,
        input_file.stream.read(entry->width * entry->height),
        palette);

    return util::file_from_grid(pixels, entry->name);
}

static auto dummy = fmt::register_fmt<EgrArchiveDecoder>("libido/egr");
