#include "dec/twilight_frontier/pak1_audio_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "util/file_from_audio.h"

using namespace au;
using namespace au::dec::twilight_frontier;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
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

bool Pak1AudioArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("dat"))
        return false;
    size_t file_count = input_file.stream.read_u32_le();
    for (auto i : algo::range(file_count))
    {
        if (!input_file.stream.read_u8())
            continue;
        auto size = input_file.stream.read_u32_le();
        input_file.stream.skip(18);
        input_file.stream.skip(size);
    }
    return input_file.stream.eof();
}

std::unique_ptr<dec::ArchiveMeta> Pak1AudioArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto file_count = input_file.stream.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        if (!input_file.stream.read_u8())
            continue;

        entry->size = input_file.stream.read_u32_le();
        entry->format = input_file.stream.read_u16_le();
        entry->channel_count = input_file.stream.read_u16_le();
        entry->sample_rate = input_file.stream.read_u32_le();
        entry->byte_rate = input_file.stream.read_u32_le();
        entry->block_align = input_file.stream.read_u16_le();
        entry->bits_per_sample = input_file.stream.read_u16_le();
        input_file.stream.skip(2);

        entry->offset = input_file.stream.tell();
        input_file.stream.skip(entry->size);
        entry->path = algo::format("%04d", i);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Pak1AudioArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    res::Audio audio;
    audio.channel_count = entry->channel_count;
    audio.bits_per_sample = entry->bits_per_sample;
    audio.sample_rate = entry->sample_rate;
    audio.samples = input_file.stream.seek(entry->offset).read(entry->size);
    return util::file_from_audio(audio, entry->path);
}

static auto _ = dec::register_decoder<Pak1AudioArchiveDecoder>(
    "twilight-frontier/pak1-sfx");
