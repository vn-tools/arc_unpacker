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

#include "dec/twilight_frontier/pak1_audio_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"
#include "enc/microsoft/wav_audio_encoder.h"

using namespace au;
using namespace au::dec::twilight_frontier;

namespace
{
    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
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
    const auto file_count = input_file.stream.read_le<u32>();
    for (const auto i : algo::range(file_count))
    {
        if (!input_file.stream.read<u8>())
            continue;
        const auto size = input_file.stream.read_le<u32>();
        input_file.stream.skip(18);
        input_file.stream.skip(size);
    }
    return input_file.stream.left() == 0;
}

std::unique_ptr<dec::ArchiveMeta> Pak1AudioArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto file_count = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        if (!input_file.stream.read<u8>())
            continue;

        entry->size = input_file.stream.read_le<u32>();
        entry->format = input_file.stream.read_le<u16>();
        entry->channel_count = input_file.stream.read_le<u16>();
        entry->sample_rate = input_file.stream.read_le<u32>();
        entry->byte_rate = input_file.stream.read_le<u32>();
        entry->block_align = input_file.stream.read_le<u16>();
        entry->bits_per_sample = input_file.stream.read_le<u16>();
        input_file.stream.skip(2);

        entry->offset = input_file.stream.pos();
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
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    res::Audio audio;
    audio.channel_count = entry->channel_count;
    audio.bits_per_sample = entry->bits_per_sample;
    audio.sample_rate = entry->sample_rate;
    audio.samples = input_file.stream.seek(entry->offset).read(entry->size);
    const auto encoder = enc::microsoft::WavAudioEncoder();
    return encoder.encode(logger, audio, entry->path);
}

static auto _ = dec::register_decoder<Pak1AudioArchiveDecoder>(
    "twilight-frontier/pak1-sfx");
