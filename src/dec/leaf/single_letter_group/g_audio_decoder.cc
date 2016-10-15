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

#include "dec/leaf/single_letter_group/g_audio_decoder.h"
#include "algo/crypt/crc32.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "bw\x20\x20"_b;

namespace
{
    enum State
    {
        StateHeader,
        StateComment,
        StatePayload,
    };
}

static void write_page(io::BaseByteStream &output_stream, bstr &page)
{
    page[0] = 'O';
    page[1] = 'g';
    page[2] = 'g';
    page[3] = 'S';
    page[0x16] = 0;
    page[0x17] = 0;
    page[0x18] = 0;
    page[0x19] = 0;
    const auto crc = algo::crypt::crc32(page);
    page[0x16] = crc >> 24;
    page[0x17] = crc >> 16;
    page[0x18] = crc >> 8;
    page[0x19] = crc;
    output_stream.write(page);
}

bool GAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("g"))
        return false;
    return input_file.stream.seek(4).read(10) == "\x00\x02\0\0\0\0\0\0\0\0"_b;
}

std::unique_ptr<io::File> GAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    auto output_file = std::make_unique<io::File>(input_file.path, ""_b);

    auto state = StateHeader;
    while (input_file.stream.left())
    {
        auto page = input_file.stream.read(0x1B);

        const auto segment_count = page[0x1A];
        if (!segment_count)
        {
            write_page(output_file->stream, page);
            continue;
        }

        page += input_file.stream.read(segment_count);

        auto segments_size = 0_z;
        for (const auto i : algo::range(segment_count))
            segments_size += page[0x1B + i];

        if (state == StateHeader)
        {
            const auto id = input_file.stream.read<u8>();
            input_file.stream.skip(1);
            page += id;
            segments_size -= 2;
            if (id != 1)
                throw err::CorruptDataError("Unexpected packet ID");
            page += "vorbis"_b;
            page[0x1B] += 5;
            state = StateComment;
        }

        else if (state == StateComment)
        {
            {
                const auto id = input_file.stream.read<u8>();
                input_file.stream.skip(1);
                page += id;
                segments_size -= 2;
                if (id != 3)
                    throw err::CorruptDataError("Unexpected packet ID");
                const auto comment_size = page[0x1B] - 2;
                page += "vorbis"_b;
                page += input_file.stream.read(comment_size);
                segments_size -= comment_size;
                page[0x1B] += 5;
            }

            {
                const auto id = input_file.stream.read<u8>();
                input_file.stream.skip(1);
                page += id;
                segments_size -= 2;
                if (id != 5)
                    throw err::CorruptDataError("Unexpected packet ID");
                page += "vorbis"_b;
                page[0x1B + segment_count - 1] += 5;
                state = StatePayload;
            }
        }

        page += input_file.stream.read(segments_size);

        write_page(output_file->stream, page);
    }

    output_file->path.change_extension("ogg");
    return output_file;
}

static auto _ = dec::register_decoder<GAudioDecoder>("leaf/g");
