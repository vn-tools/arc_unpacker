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

#include "dec/french_bread/ex3_image_decoder.h"
#include "algo/range.h"
#include "dec/microsoft/bmp_image_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::french_bread;

static const bstr magic = "LLIF"_b;

bool Ex3ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image Ex3ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    bstr data;
    bstr table0 = input_file.stream.read(0x40);
    bstr table1(256);
    bstr table2(256);

    u8 b = input_file.stream.read<u8>();
    while (input_file.stream.left())
    {
        for (const auto j : algo::range(256))
            table1[j] = j;

        size_t offset = 0;
        while (true)
        {
            if (b > 127)
            {
                offset += b - 127;
                b = 0;
            }
            if (offset == 256)
                break;
            for (u8 j : algo::range(b + 1))
            {
                if (offset >= 256)
                    throw err::BadDataOffsetError();
                table1[offset] = input_file.stream.read<u8>();
                if (offset != table1[offset])
                    table2[offset] = input_file.stream.read<u8>();
                ++offset;
            }
            if (offset == 256)
                break;
            b = input_file.stream.read<u8>();
        }

        int left = input_file.stream.read_be<u16>();
        offset = 0;
        while (true)
        {
            if (offset)
            {
                --offset;
                if (offset >= 60)
                    throw err::BadDataOffsetError();
                b = table0[offset];
            }
            else
            {
                if (!left)
                    break;
                --left;
                b = input_file.stream.read<u8>();
            }

            if (b == table1[b])
            {
                data += b;
            }
            else
            {
                if (offset >= 58)
                    throw err::BadDataOffsetError();
                table0[offset++] = table2[b];
                table0[offset++] = table1[b];
            }
        }
        if (input_file.stream.left())
            b = input_file.stream.read<u8>();
    }

    io::File bmp_file(input_file.path, data);
    const auto bmp_file_decoder = dec::microsoft::BmpImageDecoder();
    return bmp_file_decoder.decode(logger, bmp_file);
}

static auto _ = dec::register_decoder<Ex3ImageDecoder>("french-bread/ex3");
