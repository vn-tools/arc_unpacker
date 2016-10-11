// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "dec/team_shanghai_alice/thbgm_audio_archive_decoder.h"
#include "algo/range.h"
#include "algo/str.h"
#include "dec/team_shanghai_alice/pbg4_archive_decoder.h"
#include "dec/team_shanghai_alice/pbgz_archive_decoder.h"
#include "dec/team_shanghai_alice/tha1_archive_decoder.h"
#include "enc/microsoft/wav_audio_encoder.h"
#include "err.h"
#include "io/file_system.h"

using namespace au;
using namespace au::dec::team_shanghai_alice;

static const bstr magic = "ZWAV"_b;

namespace
{
    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
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
        for (const auto &decoder : decoders)
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
    while (definitions_file->stream.left())
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->path = definitions_file->stream.read_to_zero(16).str();
        if (entry->path.str().empty())
            break;
        entry->offset = definitions_file->stream.read_le<u32>();
        definitions_file->stream.skip(4);
        entry->intro_size = definitions_file->stream.read_le<u32>();
        entry->size = definitions_file->stream.read_le<u32>();
        entry->format = definitions_file->stream.read_le<u16>();
        entry->channel_count = definitions_file->stream.read_le<u16>();
        entry->sample_rate = definitions_file->stream.read_le<u32>();
        entry->byte_rate = definitions_file->stream.read_le<u32>();
        entry->block_align = definitions_file->stream.read_le<u16>();
        entry->bits_per_sample = definitions_file->stream.read_le<u16>();
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
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
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
    const auto encoder = enc::microsoft::WavAudioEncoder();
    return encoder.encode(logger, audio, entry->path);
}

static auto _ = dec::register_decoder<ThbgmAudioArchiveDecoder>(
    "team-shanghai-alice/thbgm");
