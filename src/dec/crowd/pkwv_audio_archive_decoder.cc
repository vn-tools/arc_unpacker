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

#include "dec/crowd/pkwv_audio_archive_decoder.h"
#include "algo/range.h"
#include "enc/microsoft/wav_audio_encoder.h"

using namespace au;
using namespace au::dec::crowd;

static const auto magic = "PKWV"_b;

namespace
{
    struct FormatInfo final
    {
        u16 codec;
        size_t channel_count;
        size_t sample_rate;
        size_t byte_rate;
        size_t bits_per_sample;
        size_t block_align;
    };

    struct CustomArchiveEntry final : dec::PlainArchiveEntry
    {
        FormatInfo fmt;
    };
}

bool PkwvAudioArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> PkwvAudioArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto fmt_count = input_file.stream.read_le<u16>();
    const auto file_count = input_file.stream.read_le<u16>();
    const auto data_offset = input_file.stream.pos()
        + fmt_count * 20
        + file_count * 24;

    std::vector<FormatInfo> fmt_list;
    for (const auto i : algo::range(fmt_count))
    {
        FormatInfo fmt;
        fmt.codec = input_file.stream.read_le<u16>();
        fmt.channel_count = input_file.stream.read_le<u16>();
        fmt.sample_rate = input_file.stream.read_le<u32>();
        fmt.byte_rate = input_file.stream.read_le<u32>();
        fmt.bits_per_sample = input_file.stream.read_le<u16>();
        fmt.block_align = input_file.stream.read_le<u16>();
        input_file.stream.skip(4);
        fmt_list.push_back(fmt);
    }

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<CustomArchiveEntry>();
        entry->fmt = fmt_list.at(input_file.stream.read_le<u16>());
        entry->path = input_file.stream.read_to_zero(10).str();
        entry->size = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.read_le<u64>() + data_offset;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> PkwvAudioArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const CustomArchiveEntry*>(&e);
    res::Audio audio;
    audio.codec = entry->fmt.codec;
    audio.channel_count = entry->fmt.channel_count;
    audio.sample_rate = entry->fmt.sample_rate;
    audio.bits_per_sample = entry->fmt.bits_per_sample;
    audio.samples = input_file.stream.seek(entry->offset).read(entry->size);
    const auto encoder = enc::microsoft::WavAudioEncoder();
    return encoder.encode(logger, audio, entry->path);
}

static auto _ = dec::register_decoder<PkwvAudioArchiveDecoder>("crowd/pkwv");
