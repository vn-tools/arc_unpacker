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

#include "dec/fc01/acd_image_decoder.h"
#include "algo/range.h"
#include "dec/fc01/common/custom_lzss.h"
#include "err.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::fc01;

static const bstr magic = "ACD 1.00"_b;

static bstr do_decode(const bstr &input, size_t canvas_size)
{
    io::MsbBitStream bit_stream(input);
    bstr output(canvas_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();
    while (output_ptr < output_end)
    {
        s32 byte = 0;
        if (bit_stream.read(1))
        {
            byte--;
            if (!bit_stream.read(1))
            {
                byte += 3;

                int bit = 0;
                while (!bit)
                {
                    bit = bit_stream.read(1);
                    byte = (byte << 1) | bit;
                    bit = (byte >> 8) & 1;
                    byte &= 0xFF;
                }

                if (byte)
                {
                    byte++;
                    byte *= 0x28CCCCD;
                    byte >>= 24;
                }
            }
        }
        *output_ptr++ = byte;
    }
    return output;
}

bool AcdImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image AcdImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    const auto data_offset = input_file.stream.read_le<u32>();
    const auto size_comp = input_file.stream.read_le<u32>();
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();

    input_file.stream.seek(data_offset);
    auto pixel_data = input_file.stream.read(size_comp);
    pixel_data = common::custom_lzss_decompress(pixel_data, size_orig);
    pixel_data = do_decode(pixel_data, width * height);

    return res::Image(width, height, pixel_data, res::PixelFormat::Gray8);
}

static auto _ = dec::register_decoder<AcdImageDecoder>("fc01/acd");
