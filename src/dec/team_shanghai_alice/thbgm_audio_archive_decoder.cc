#include "dec/team_shanghai_alice/thbgm_audio_archive_decoder.h"
#include "algo/range.h"
#include "algo/str.h"
#include "dec/team_shanghai_alice/pbg4_archive_decoder.h"
#include "dec/team_shanghai_alice/pbgz_archive_decoder.h"
#include "dec/team_shanghai_alice/tha1_archive_decoder.h"
#include "err.h"
#include "io/file_system.h"
#include "util/file_from_audio.h"

using namespace au;
using namespace au::dec::team_shanghai_alice;

static const bstr magic = "ZWAV"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
        size_t intro_size;
        size_t format;
        size_t channel_count;
        size_t sample_rate;
        size_t byte_rate;
        size_t block_align;
        size_t bits_per_sample;
    };
}

static std::unique_ptr<io::File> grab_definitions_file(
    const Logger &logger, const io::path &dir)
{
    std::vector<std::unique_ptr<dec::BaseArchiveDecoder>> decoders;
    decoders.push_back(std::make_unique<Pbg4ArchiveDecoder>());
    decoders.push_back(std::make_unique<PbgzArchiveDecoder>());
    decoders.push_back(std::make_unique<Tha1ArchiveDecoder>());

    for (const auto &path : io::directory_range(dir))
    {
        if (!io::is_regular_file(path))
            continue;

        io::File other_file(path, io::FileMode::Read);
        for (auto &decoder : decoders)
        {
            if (!decoder->is_recognized(other_file))
                continue;

            auto meta = decoder->read_meta(logger, other_file);
            for (auto &entry : meta->entries)
            {
                if (entry->path.name() != "thbgm.fmt")
                    continue;
                auto output_file = decoder->read_file(
                    logger, other_file, *meta, *entry);
                output_file->stream.seek(0);
                return output_file;
            }
        }
    }

    return nullptr;
}

bool ThbgmAudioArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> ThbgmAudioArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto dir = input_file.path.parent();
    auto definitions_file = grab_definitions_file(logger, dir);

    if (!definitions_file)
        throw err::NotSupportedError("BGM definitions not found");

    auto meta = std::make_unique<ArchiveMeta>();
    while (!definitions_file->stream.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = definitions_file->stream.read_to_zero(16).str();
        if (entry->path.str().empty())
            break;
        entry->offset = definitions_file->stream.read_u32_le();
        definitions_file->stream.skip(4);
        entry->intro_size = definitions_file->stream.read_u32_le();
        entry->size = definitions_file->stream.read_u32_le();
        entry->format = definitions_file->stream.read_u16_le();
        entry->channel_count = definitions_file->stream.read_u16_le();
        entry->sample_rate = definitions_file->stream.read_u32_le();
        entry->byte_rate = definitions_file->stream.read_u32_le();
        entry->block_align = definitions_file->stream.read_u16_le();
        entry->bits_per_sample = definitions_file->stream.read_u16_le();
        definitions_file->stream.skip(4);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> ThbgmAudioArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto samples = input_file.stream
        .seek(entry->offset)
        .read(entry->size);

    res::Audio audio;
    audio.channel_count = entry->channel_count;
    audio.bits_per_sample = entry->bits_per_sample;
    audio.sample_rate = entry->sample_rate;
    audio.samples = samples;
    audio.loops.push_back(res::AudioLoopInfo
        {entry->intro_size, entry->size - entry->intro_size, 0});
    return util::file_from_audio(audio, entry->path);
}

static auto _ = dec::register_decoder<ThbgmAudioArchiveDecoder>(
    "team-shanghai-alice/thbgm");
