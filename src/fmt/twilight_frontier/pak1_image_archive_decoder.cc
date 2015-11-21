#include "fmt/twilight_frontier/pak1_image_archive_decoder.h"
#include "err.h"
#include "io/memory_stream.h"
#include "util/file_from_grid.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

namespace
{
    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        std::vector<pix::Palette> palettes;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        size_t width, height;
        size_t depth;
    };
}

bool Pak1ImageArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    if (!arc_file.has_extension("dat"))
        return false;
    auto palette_count = arc_file.stream.read_u8();
    arc_file.stream.skip(palette_count * 512);
    while (!arc_file.stream.eof())
    {
        arc_file.stream.skip(4 * 3);
        arc_file.stream.skip(1);
        arc_file.stream.skip(arc_file.stream.read_u32_le());
    }
    return arc_file.stream.eof();
}

std::unique_ptr<fmt::ArchiveMeta>
    Pak1ImageArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto meta = std::make_unique<ArchiveMetaImpl>();
    auto palette_count = arc_file.stream.read_u8();
    for (auto i : util::range(palette_count))
    {
        meta->palettes.push_back(pix::Palette(
            256, arc_file.stream.read(512), pix::Format::BGRA5551));
    }
    meta->palettes.push_back(pix::Palette(256));

    size_t i = 0;
    while (arc_file.stream.tell() < arc_file.stream.size())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->width = arc_file.stream.read_u32_le();
        entry->height = arc_file.stream.read_u32_le();
        arc_file.stream.skip(4);
        entry->depth = arc_file.stream.read_u8();
        entry->size = arc_file.stream.read_u32_le();
        entry->name = util::format("%04d", i++);
        entry->offset = arc_file.stream.tell();
        arc_file.stream.skip(entry->size);
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<File> Pak1ImageArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);

    auto chunk_size = 0;
    if (entry->depth == 32 || entry->depth == 24)
        chunk_size = 4;
    else if (entry->depth == 16)
        chunk_size = 2;
    else if (entry->depth == 8)
        chunk_size = 1;
    else
        throw err::UnsupportedBitDepthError(entry->depth);

    bstr output(entry->width * entry->height * chunk_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<u8>();
    arc_file.stream.seek(entry->offset);
    io::MemoryStream input(arc_file.stream, entry->size);

    while (output_ptr < output_end && input.tell() < input.size())
    {
        size_t repeat;
        if (entry->depth == 32 || entry->depth == 24)
            repeat = input.read_u32_le();
        else if (entry->depth == 16)
            repeat = input.read_u16_le();
        else if (entry->depth == 8)
            repeat = input.read_u8();
        else
            throw err::UnsupportedBitDepthError(entry->depth);

        auto chunk = input.read(chunk_size);
        while (repeat--)
            for (auto &c : chunk)
                *output_ptr++ = c;
    }

    std::unique_ptr<pix::Grid> pixels;
    if (entry->depth == 32)
    {
        pixels = std::make_unique<pix::Grid>(
            entry->width, entry->height, output, pix::Format::BGRA8888);
    }
    else if (entry->depth == 24)
    {
        pixels = std::make_unique<pix::Grid>(
            entry->width, entry->height, output, pix::Format::BGR888X);
    }
    else if (entry->depth == 16)
    {
        pixels = std::make_unique<pix::Grid>(
            entry->width, entry->height, output, pix::Format::BGRA5551);
    }
    else if (entry->depth == 8)
    {
        pixels = std::make_unique<pix::Grid>(
            entry->width, entry->height, output, meta->palettes[0]);
    }
    else
        throw err::UnsupportedBitDepthError(entry->depth);

    return util::file_from_grid(*pixels, entry->name);
}

static auto dummy
    = fmt::register_fmt<Pak1ImageArchiveDecoder>("twilight-frontier/pak1-gfx");
