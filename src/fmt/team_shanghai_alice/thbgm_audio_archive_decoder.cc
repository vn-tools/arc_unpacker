#include "fmt/team_shanghai_alice/thbgm_audio_archive_decoder.h"
#include <boost/lexical_cast.hpp>
#include "err.h"
#include "fmt/team_shanghai_alice/pbg4_archive_decoder.h"
#include "fmt/team_shanghai_alice/pbgz_archive_decoder.h"
#include "fmt/team_shanghai_alice/tha1_archive_decoder.h"
#include "io/filesystem.h"
#include "util/file_from_audio.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::team_shanghai_alice;

static const bstr magic = "ZWAV"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
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

static std::unique_ptr<io::File> grab_definitions_file(const io::path &dir)
{
    std::vector<std::unique_ptr<fmt::ArchiveDecoder>> decoders;
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

            auto meta = decoder->read_meta(other_file);
            for (auto &entry : meta->entries)
            {
                if (entry->name.find("thbgm.fmt") == std::string::npos)
                    continue;
                auto output_file = decoder->read_file(
                    other_file, *meta, *entry);
                output_file->stream.seek(0);
                return output_file;
            }
        }
    }

    return nullptr;
}

struct ThbgmAudioArchiveDecoder::Priv final
{
    size_t loop_count = 5;
};

ThbgmAudioArchiveDecoder::ThbgmAudioArchiveDecoder() : p(new Priv())
{
}

ThbgmAudioArchiveDecoder::~ThbgmAudioArchiveDecoder()
{
}

void ThbgmAudioArchiveDecoder::register_cli_options(ArgParser &arg_parser) const
{
    auto sw = arg_parser.register_switch({"--loops"})
        ->set_value_name("NUM")
        ->set_description("Number of BGM loops (default: 5)");
    ArchiveDecoder::register_cli_options(arg_parser);
}

void ThbgmAudioArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    if (arg_parser.has_switch("loops"))
    {
        set_loop_count(
            boost::lexical_cast<int>(arg_parser.get_switch("loops")));
    }
    ArchiveDecoder::parse_cli_options(arg_parser);
}

void ThbgmAudioArchiveDecoder::set_loop_count(const size_t loop_count)
{
    p->loop_count = loop_count;
}

bool ThbgmAudioArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    ThbgmAudioArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    auto dir = input_file.name.parent();
    auto definitions_file = grab_definitions_file(dir);

    if (!definitions_file)
        throw err::NotSupportedError("BGM definitions not found");

    auto meta = std::make_unique<ArchiveMeta>();
    while (!definitions_file->stream.eof())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = definitions_file->stream.read_to_zero(16).str();
        if (entry->name.empty())
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
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto samples = input_file.stream.read(entry->intro_size);
    for (auto i : util::range(p->loop_count))
    {
        input_file.stream.seek(entry->offset + entry->intro_size);
        samples += input_file.stream.read(entry->size - entry->intro_size);
    }

    res::Audio audio;
    audio.channel_count = entry->channel_count;
    audio.bits_per_sample = entry->bits_per_sample;
    audio.sample_rate = entry->sample_rate;
    audio.samples = samples;
    return util::file_from_audio(audio, entry->name);
}

static auto dummy
    = fmt::register_fmt<ThbgmAudioArchiveDecoder>("team-shanghai-alice/thbgm");
