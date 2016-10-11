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

#include "dec/escude/acp_file_decoder.h"
#include <stack>
#include "algo/range.h"
#include "err.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::escude;

static const bstr magic = "acp\x00"_b;

namespace
{
    struct Something final
    {
        u32 d0, d1;
    };
}

bool AcpFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> AcpFileDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    io::MsbBitStream bit_stream(input_file.stream);
    const auto size_orig = bit_stream.read(32);

    bstr data;
    while (data.size() < size_orig)
    {
        std::vector<Something> somethings(35023);
        for (auto &something : somethings)
        {
            something.d0 = 0;
            something.d1 = 0;
        }

        size_t marker = 259;
        size_t chunk_size = 9;
        auto control0 = bit_stream.read(chunk_size);
        auto control1 = control0;
        if (control0 == 256)
            continue;

        data += static_cast<u8>(control0);
        while (data.size() < size_orig)
        {
            const auto control2 = bit_stream.read(chunk_size);
            if (control2 == 257)
                chunk_size++;
            else if (control2 == 258)
                break;
            else if (control2 == 256)
                break;
            else
            {
                std::stack<u8> stack;
                auto control3 = control2;
                if (control3 >= marker)
                {
                    control3 = control0;
                    stack.push(control1);
                }
                while (control3 > 0xFF)
                {
                    const auto &something = somethings.at(control3);
                    control3 = something.d0;
                    stack.push(something.d1);
                }
                stack.push(control3);
                if (data.size() + stack.size() > size_orig)
                    throw err::BadDataSizeError();
                while (stack.size())
                {
                    data += stack.top();
                    stack.pop();
                }
                auto &something = somethings.at(marker++);
                something.d0 = control0;
                something.d1 = control3;
                control0 = control2;
                control1 = control3;
            }
        }
    }

    return std::make_unique<io::File>(input_file.path, data);
}

static auto _ = dec::register_decoder<AcpFileDecoder>("escude/acp");
