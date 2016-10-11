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

#include "dec/aoi/agf_image_decoder.h"
#include "algo/ptr.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::aoi;

static const bstr magic = "AGF\x00"_b;

bool AgfImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image AgfImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto version = input_file.stream.read_le<u32>();
    const auto file_size = input_file.stream.read_le<u32>();
    const auto data_offset = input_file.stream.read_le<u32>();
    input_file.stream.skip(12);
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();

    if (version != 1)
        throw err::UnsupportedVersionError(version);
    if (file_size != input_file.stream.size())
        throw err::CorruptDataError("Expected file size");

    const auto input = input_file.stream.seek(data_offset).read_to_eof();
    bstr output(width * height * 4);
    auto input_ptr = algo::make_ptr(input.get<u32>(), input.size() / 4);
    auto output_ptr = algo::make_ptr(output.get<u32>(), output.size() / 4);
    while (input_ptr.left() && output_ptr.left())
    {
        const auto control = *input_ptr++;
        const auto strategy = control & 0xFF;
        if (strategy == 1)
        {
            const auto repetitions = control >> 8;
            output_ptr.append_from(input_ptr, repetitions);
        }
        else if (strategy == 2)
        {
            const auto repetitions = control >> 8;
            output_ptr.append_basic(*input_ptr++, repetitions);
        }
        else if (strategy == 3)
        {
            const auto repetitions = control >> 16;
            for (const auto i : algo::range((control >> 8) & 0xFF))
            {
                auto chunk_ptr = input_ptr;
                output_ptr.append_from(chunk_ptr, repetitions);
            }
            input_ptr += repetitions;
        }
        else if (strategy == 4)
        {
            const auto source_pos = -((control >> 8) & 0xFFF);
            const auto repetitions = control >> 20;
            output_ptr.append_self(source_pos, repetitions);
        }
    }
    return res::Image(width, height, output, res::PixelFormat::BGRA8888);
}

static auto _ = dec::register_decoder<AgfImageDecoder>("aoi/agf");
