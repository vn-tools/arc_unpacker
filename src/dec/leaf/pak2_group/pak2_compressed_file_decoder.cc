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

#include "dec/leaf/pak2_group/pak2_compressed_file_decoder.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::leaf;

static const bstr magic = "\xAF\xF6\x4D\x4F"_b;

static bstr decompress(const bstr &src, const size_t size_orig)
{
    bstr output;
    output.reserve(size_orig);
    io::MemoryByteStream input_stream(src);
    while (output.size() < size_orig && input_stream.left())
    {
        const auto control = input_stream.read<u8>();
        if (control >= 0x80)
        {
            if (control >= 0xA0)
            {
                int repeat;
                int base_value;
                if (control == 0xFF)
                {
                    repeat = input_stream.read<u8>() + 2;
                    base_value = 0;
                }
                else if (control >= 0xE0)
                {
                    repeat = (control & 0x1F) + 2;
                    base_value = 0;
                }
                else
                {
                    repeat = (control & 0x1F) + 2;
                    base_value = input_stream.read<u8>();
                }
                output += bstr(repeat, base_value);
            }
            else
            {
                const auto repeat = control & 0x1F;
                output += input_stream.read(repeat);
            }
        }
        else
        {
            const auto look_behind
                = (input_stream.read<u8>() + (control << 8)) % 0x400;
            const auto repeat = (control >> 2) + 2;
            for (const auto i : algo::range(repeat))
                output += output[output.size() - look_behind];
        }
    }
    return output;
}

bool Pak2CompressedFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(4).read(magic.size()) == magic;
}

std::unique_ptr<io::File> Pak2CompressedFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto size_orig = input_file.stream.seek(24).read_le<u32>();
    const auto data = input_file.stream.seek(36).read_to_eof();
    return std::make_unique<io::File>(
        input_file.path, decompress(data, size_orig));
}

static auto _ = dec::register_decoder<Pak2CompressedFileDecoder>(
    "leaf/pak2-compressed-file");
