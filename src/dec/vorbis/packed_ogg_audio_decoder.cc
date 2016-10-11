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

#include "dec/vorbis/packed_ogg_audio_decoder.h"
#include "algo/range.h"
#include "err.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::vorbis;

namespace
{
    struct OggPage final
    {
        u8 version;
        u8 header_type;
        bstr granule_position;
        u32 bitstream_serial_number;
        u32 page_sequence_number;
        u32 checksum;
        std::vector<bstr> segments;
    };
}

static const bstr ogg_magic = "OggS"_b;

static OggPage read_ogg_page(io::BaseByteStream &input_stream)
{
    OggPage page;
    page.version = input_stream.read<u8>();
    page.header_type = input_stream.read<u8>();
    page.granule_position = input_stream.read(8);
    page.bitstream_serial_number = input_stream.read_le<u32>();
    page.page_sequence_number = input_stream.read_le<u32>();
    page.checksum = input_stream.read_le<u32>();
    std::vector<size_t> sizes(input_stream.read<u8>());
    for (auto &size : sizes)
        size = input_stream.read<u8>();
    for (auto &size : sizes)
        page.segments.push_back(input_stream.read(size));
    return page;
}

static void write_ogg_page(
    io::BaseByteStream &output_stream, const OggPage &page)
{
    output_stream.write(ogg_magic);
    output_stream.write<u8>(page.version);
    output_stream.write<u8>(page.header_type);
    output_stream.write(page.granule_position);
    output_stream.write_le<u32>(page.bitstream_serial_number);
    output_stream.write_le<u32>(page.page_sequence_number);
    output_stream.write_le<u32>(page.checksum);
    output_stream.write<u8>(page.segments.size());
    for (auto &page_segment : page.segments)
        output_stream.write<u8>(page_segment.size());
    for (auto &page_segment : page.segments)
        output_stream.write(page_segment);
}

bool PackedOggAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("wav"))
        return false;
    input_file.stream.seek(16);
    input_file.stream.skip(input_file.stream.read_le<u32>());
    input_file.stream.skip(20);
    return input_file.stream.read(4) == ogg_magic;
}

static void rewrite_ogg_stream(
    const Logger &logger,
    io::BaseByteStream &input_stream,
    io::BaseByteStream &output_stream)
{
    // The OGG files used by LiarSoft may contain multiple streams, out of
    // which only the first one contains actual audio data.

    u32 initial_serial_number = 0;
    auto pages = 0;
    auto serial_number_known = false;
    while (input_stream.left())
    {
        OggPage page;
        try
        {
            if (input_stream.read(4) != ogg_magic)
                throw err::CorruptDataError("Expected OGG signature");

            page = read_ogg_page(input_stream);
        }
        catch (const err::IoError)
        {
            logger.warn(
                "Last OGG page is truncated; recovered %d pages.\n", pages);
            break;
        }

        if (!serial_number_known)
        {
            initial_serial_number = page.bitstream_serial_number;
            serial_number_known = true;
        }

        // The extra streams cause problems with popular (notably, all
        // ffmpeg-based) audio players, so we discard these streams here.
        if (page.bitstream_serial_number == initial_serial_number)
        {
            write_ogg_page(output_stream, page);
            pages++;
        }
    }
}

std::unique_ptr<io::File> PackedOggAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    if (input_file.stream.read(4) != "RIFF"_b)
        throw err::CorruptDataError("Expected RIFF signature");
    input_file.stream.skip(4); // file size w/o RIFF header, usually corrupted
    if (input_file.stream.read(8) != "WAVEfmt\x20"_b)
        throw err::CorruptDataError("Expected WAVE header");
    input_file.stream.skip(input_file.stream.read_le<u32>());

    if (input_file.stream.read(4) != "fact"_b)
        throw err::CorruptDataError("Expected fact chunk");
    if (input_file.stream.read_le<u32>() != 4)
        throw err::CorruptDataError("Corrupt fact chunk");
    input_file.stream.skip(4);

    if (input_file.stream.read(4) != "data"_b)
        throw err::CorruptDataError("Expected data chunk");
    const auto data_size = input_file.stream.read_le<u32>();
    io::MemoryByteStream input(input_file.stream, data_size);

    auto output_file = std::make_unique<io::File>();
    rewrite_ogg_stream(logger, input, output_file->stream);
    output_file->path = input_file.path;
    output_file->guess_extension();
    return output_file;
}

static auto _ = dec::register_decoder<PackedOggAudioDecoder>("vorbis/wav");
