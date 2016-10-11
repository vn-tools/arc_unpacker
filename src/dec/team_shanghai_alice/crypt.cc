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

#include "dec/team_shanghai_alice/crypt.h"
#include "algo/range.h"
#include "io/memory_byte_stream.h"

using namespace au;
using namespace au::dec::team_shanghai_alice;

bool DecryptorContext::operator ==(const DecryptorContext &other) const
{
    return key == other.key
        && step == other.step
        && block_size == other.block_size
        && limit == other.limit;
}

static void decrypt(
    io::BaseByteStream &input_stream,
    io::BaseByteStream &output_stream,
    const DecryptorContext &context)
{
    int left = input_stream.size();
    size_t current_block_size = context.block_size;
    u8 key = context.key;

    size_t shift = left % current_block_size;
    if (shift >= (current_block_size >> 2))
        shift = 0;
    shift += (left & 1);
    left -= shift;

    while (left > 0 && output_stream.pos() < context.limit)
    {
        if (left < static_cast<int>(current_block_size))
            current_block_size = left;
        auto input_block = input_stream.read(current_block_size);
        auto *input_block_ptr = input_block.get<const char>();
        bstr output_block(current_block_size);
        for (const auto j : algo::range(2))
        {
            u8 *output_block_ptr = &output_block[current_block_size - j - 1];
            for (const auto i : algo::range((current_block_size - j + 1) >> 1))
            {
                *output_block_ptr = *input_block_ptr++ ^ key;
                output_block_ptr -= 2;
                key += context.step;
            }
        }

        output_stream.write(output_block);
        left -= current_block_size;
    }

    left += shift;
    left = std::min(left, static_cast<int>(
        input_stream.size() - input_stream.pos()));
    if (left > 0)
        output_stream.write(input_stream.read(left));
    output_stream.seek(0);
}

bstr au::dec::team_shanghai_alice::decrypt(
    const bstr &input, const DecryptorContext &context)
{
    io::MemoryByteStream input_stream(input);
    io::MemoryByteStream output_stream;
    ::decrypt(input_stream, output_stream, context);
    return output_stream.read_to_eof();
}
