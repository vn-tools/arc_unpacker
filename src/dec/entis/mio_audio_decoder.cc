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

#include "dec/entis/mio_audio_decoder.h"
#include "algo/format.h"
#include "dec/entis/audio/lossless.h"
#include "dec/entis/audio/lossy.h"
#include "dec/entis/common/enums.h"
#include "dec/entis/common/sections.h"
#include "err.h"

using namespace au;
using namespace au::dec::entis;

static const bstr magic1 = "Entis\x1A\x00\x00"_b;
static const bstr magic2 = "\x00\x01\x00\x03\x00\x00\x00\x00"_b;
static const bstr magic3 = "Music Interleaved and Orthogonal"_b;

static audio::MioHeader read_header(
    io::BaseByteStream &input_stream,
    const common::SectionReader &section_reader)
{
    auto header_section = section_reader.get_section("Header");
    input_stream.seek(header_section.data_offset);
    common::SectionReader header_section_reader(input_stream);
    header_section = header_section_reader.get_section("SoundInf");
    input_stream.seek(header_section.data_offset);

    audio::MioHeader header;
    header.version = input_stream.read_le<u32>();
    header.transformation = input_stream.read_le<common::Transformation>();
    header.architecture = input_stream.read_le<common::Architecture>();

    header.channel_count   = input_stream.read_le<u32>();
    header.sample_rate     = input_stream.read_le<u32>();
    header.blockset_count  = input_stream.read_le<u32>();
    header.subband_degree  = input_stream.read_le<u32>();
    header.sample_count    = input_stream.read_le<u32>();
    header.lapped_degree   = input_stream.read_le<u32>();
    header.bits_per_sample = input_stream.read_le<u32>();
    return header;
}

static std::vector<audio::MioChunk> read_chunks(
    io::BaseByteStream &input_stream, common::SectionReader &section_reader)
{
    const auto stream_section = section_reader.get_section("Stream");
    input_stream.seek(stream_section.data_offset);
    common::SectionReader chunk_section_reader(input_stream);
    std::vector<audio::MioChunk> chunks;
    for (const auto &chunk_section
        : chunk_section_reader.get_sections("SoundStm"))
    {
        input_stream.seek(chunk_section.data_offset);
        audio::MioChunk chunk;
        chunk.version = input_stream.read<u8>();
        chunk.initial = input_stream.read<u8>() > 0;
        input_stream.skip(2);
        chunk.sample_count = input_stream.read_le<u32>();
        chunk.data = input_stream.read(chunk_section.size - 8);
        chunks.push_back(chunk);
    }
    return chunks;
}

bool MioAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic1.size()) == magic1
        && input_file.stream.read(magic2.size()) == magic2
        && input_file.stream.read(magic3.size()) == magic3;
}

res::Audio MioAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0x40);

    common::SectionReader section_reader(input_file.stream);
    const auto header = read_header(input_file.stream, section_reader);
    const auto chunks = read_chunks(input_file.stream, section_reader);

    std::unique_ptr<audio::BaseAudioDecoder> impl;
    if (header.transformation == common::Transformation::Lossless)
        impl = std::make_unique<audio::LosslessAudioDecoder>(header);
    else if (header.transformation == common::Transformation::Lot)
        impl = std::make_unique<audio::LossyAudioDecoder>(header);
    else if (header.transformation == common::Transformation::LotMss)
        impl = std::make_unique<audio::LossyAudioDecoder>(header);

    if (!impl)
    {
        throw err::NotSupportedError(algo::format(
            "Transformation type %d not supported", header.transformation));
    }

    bstr samples;
    for (const auto chunk : chunks)
        samples += impl->process_chunk(chunk);

    res::Audio audio;
    audio.channel_count = header.channel_count;
    audio.bits_per_sample = header.bits_per_sample;
    audio.sample_rate = header.sample_rate;
    audio.samples = samples;
    return audio;
}

static auto _ = dec::register_decoder<MioAudioDecoder>("entis/mio");
