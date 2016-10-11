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

#include "dec/lilim/dwv_audio_decoder.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::lilim;

static const bstr magic = "DW"_b;

bool DwvAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("dwv")
        && input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Audio DwvAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 2);
    const auto header_size = input_file.stream.read_le<u32>();
    const auto samples_size = input_file.stream.read_le<u32>();
    const auto header = input_file.stream.read(header_size);
    const auto samples = input_file.stream.read(samples_size);

    res::Audio audio;
    audio.samples = samples;

    io::MemoryByteStream header_stream(header);
    audio.codec = header_stream.read_le<u16>();
    audio.channel_count = header_stream.read_le<u16>();
    audio.sample_rate = header_stream.read_le<u32>();
    const auto byte_rate = header_stream.read_le<u32>();
    const auto block_align = header_stream.read_le<u16>();
    audio.bits_per_sample = header_stream.read_le<u16>();
    if (header_stream.left())
    {
        const auto extra_data_size = header_stream.read_le<u16>();
        audio.extra_codec_headers = header_stream.read(extra_data_size);
    }

    return audio;
}

static auto _ = dec::register_decoder<DwvAudioDecoder>("lilim/dwv");
