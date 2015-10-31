#include "fmt/twilight_frontier/pak1_audio_archive_decoder.h"
#include "util/file_from_samples.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::twilight_frontier;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        size_t format;
        size_t channel_count;
        size_t sample_rate;
        size_t byte_rate;
        size_t block_align;
        size_t bits_per_sample;
    };
}

bool Pak1AudioArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    if (!arc_file.has_extension("dat"))
        return false;
    size_t file_count = arc_file.io.read_u32_le();
    for (auto i : util::range(file_count))
    {
        if (!arc_file.io.read_u8())
            continue;
        auto size = arc_file.io.read_u32_le();
        arc_file.io.skip(18);
        arc_file.io.skip(size);
    }
    return arc_file.io.eof();
}

std::unique_ptr<fmt::ArchiveMeta>
    Pak1AudioArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto file_count = arc_file.io.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        if (!arc_file.io.read_u8())
            continue;

        entry->size = arc_file.io.read_u32_le();
        entry->format = arc_file.io.read_u16_le();
        entry->channel_count = arc_file.io.read_u16_le();
        entry->sample_rate = arc_file.io.read_u32_le();
        entry->byte_rate = arc_file.io.read_u32_le();
        entry->block_align = arc_file.io.read_u16_le();
        entry->bits_per_sample = arc_file.io.read_u16_le();
        arc_file.io.skip(2);

        entry->offset = arc_file.io.tell();
        arc_file.io.skip(entry->size);
        entry->name = util::format("%04d", i);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> Pak1AudioArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    return util::file_from_samples(
        entry->channel_count,
        entry->bits_per_sample,
        entry->sample_rate,
        arc_file.io.read(entry->size),
        entry->name);
}

static auto dummy
    = fmt::register_fmt<Pak1AudioArchiveDecoder>("twilight-frontier/pak1-sfx");
