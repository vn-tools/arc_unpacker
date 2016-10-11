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

#include "dec/ivory/prs_image_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::ivory;

static const bstr magic = "YB"_b;

static bstr decode_pixels(const bstr &source, size_t width, size_t height)
{
    bstr target(width * height * 3);
    u8 *target_ptr = target.get<u8>();
    u8 *target_end = target_ptr + target.size();
    const u8 *source_ptr = source.get<const u8>();
    const u8 *source_end = source_ptr + source.size();

    int flag = 0;
    int size_lookup[256];
    for (const auto i : algo::range(256))
        size_lookup[i] = i + 3;
    size_lookup[0xFF] = 0x1000;
    size_lookup[0xFE] = 0x400;
    size_lookup[0xFD] = 0x100;

    while (source_ptr < source_end && target_ptr < target_end)
    {
        flag <<= 1;
        if ((flag & 0xFF) == 0)
        {
            flag = *source_ptr++;
            flag <<= 1;
            flag += 1;
        }

        if ((flag & 0x100) != 0x100)
        {
            *target_ptr++ = *source_ptr++;
        }
        else
        {
            int tmp = *source_ptr++;
            size_t size = 0;
            size_t shift = 0;

            if (tmp & 0x80)
            {
                shift = (*source_ptr++) | ((tmp & 0x3F) << 8);
                if (tmp & 0x40)
                {
                    if (source_ptr >= source_end)
                        break;
                    auto index = static_cast<size_t>(*source_ptr++);
                    size = size_lookup[index];
                }
                else
                {
                    size = (shift & 0xF) + 3;
                    shift >>= 4;
                }
            }
            else
            {
                size = tmp >> 2;
                tmp &= 3;
                if (tmp == 3)
                {
                    size += 9;
                    for (const auto i : algo::range(size))
                    {
                        if (source_ptr >= source_end
                            || target_ptr >= target_end)
                        {
                            break;
                        }
                        *target_ptr++ = *source_ptr++;
                    }
                    continue;
                }
                shift = size;
                size = tmp + 2;
            }

            shift += 1;
            for (const auto i : algo::range(size))
            {
                if (target_ptr >= target_end)
                    break;
                if (target_ptr - shift < target.get<u8>())
                    throw err::BadDataOffsetError();
                *target_ptr = *(target_ptr - shift);
                target_ptr++;
            }
        }
    }
    return target;
}

bool PrsImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image PrsImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());

    const bool using_differences = input_file.stream.read<u8>() > 0;
    const auto version = input_file.stream.read<u8>();
    if (version != 3)
        throw err::UnsupportedVersionError(version);

    const auto source_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();

    auto target = input_file.stream.read(source_size);
    target = decode_pixels(target, width, height);
    if (using_differences)
        for (const auto i : algo::range(3, target.size()))
            target[i] += target[i - 3];

    return res::Image(width, height, target, res::PixelFormat::BGR888);
}

static auto _ = dec::register_decoder<PrsImageDecoder>("ivory/prs");
