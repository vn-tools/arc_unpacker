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

#include "dec/kaguya/common/rle.h"
#include "algo/range.h"
#include "dec/kaguya/common/rle_test.h"
#include "io/memory_byte_stream.h"
#include "test_support/catch.h"

using namespace au;

bstr tests::dec::kaguya::common::compress_rle(
    const bstr &input, const size_t band)
{
    io::MemoryByteStream output_stream;
    for (const auto i : algo::range(band))
    {
        io::MemoryByteStream input_stream((input.size() + (band - 1)) / band);
        for (const auto j : algo::range(input.size() / band))
            input_stream.write<u8>(input.at(i + j * band));
        input_stream.seek(0);
        const auto init_b = input_stream.read<u8>();
        auto last_b = init_b;
        output_stream.write<u8>(init_b);
        while (input_stream.left())
        {
            auto b = input_stream.read<u8>();
            output_stream.write<u8>(b);
            if (last_b == b)
            {
                auto repetitions = 0;
                while (input_stream.left() && repetitions < 0x7FFF)
                {
                    if (input_stream.read<u8>() != b)
                    {
                        input_stream.skip(-1);
                        break;
                    }
                    repetitions++;
                }

                if (repetitions < 0x80)
                {
                    output_stream.write<u8>(repetitions);
                }
                else
                {
                    output_stream.write<u8>(((repetitions - 0x80) >> 8) | 0x80);
                    output_stream.write<u8>((repetitions - 0x80));
                }

                if (input_stream.left())
                {
                    b = input_stream.read<u8>();
                    output_stream.write<u8>(b);
                }
            }
            last_b = b;
        }
    }
    return output_stream.seek(0).read_to_eof();
}

static void test(const bstr &input, const size_t bands)
{
    const auto compressed
        = tests::dec::kaguya::common::compress_rle(input, bands);
    const auto decompressed
        = dec::kaguya::common::decompress_rle(compressed, input.size(), bands);
    REQUIRE(decompressed.str() == input.str());
}

TEST_CASE("Atelier Kaguya RLE compression", "[dec]")
{
    test("abc"_b, 3);
    test("abc"_b, 3);
    test("aabaab"_b, 3);
    test("aaaaaabb"_b, 4);
}
