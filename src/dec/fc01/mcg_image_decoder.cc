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

#include "dec/fc01/mcg_image_decoder.h"
#include <cstdlib>
#include "algo/format.h"
#include "algo/range.h"
#include "algo/str.h"
#include "dec/fc01/common/custom_lzss.h"
#include "dec/fc01/common/mrg_decryptor.h"
#include "dec/fc01/common/util.h"
#include "err.h"

using namespace au;
using namespace au::dec::fc01;

static const bstr magic = "MCG "_b;

static bstr decrypt_v101(const bstr &input, size_t output_size, u8 initial_key)
{
    auto data = input;
    auto key = initial_key;
    for (const auto i : algo::range(data.size()))
    {
        data[i] = common::rol8(data[i], 1) ^ key;
        key += data.size() - 1 - i;
    }
    return common::custom_lzss_decompress(data, output_size);
}

static bstr transform_v200(
    bstr planes[3], const size_t width, const size_t height)
{
    for (const auto y : algo::range(height - 1))
    for (const auto x : algo::range(width - 1))
    for (const auto i : algo::range(3))
    {
        const auto pos = y * width + x;
        int p00 = planes[i][pos];
        int p10 = planes[i][pos + width] - p00;
        int p01 = planes[i][pos + 1] - p00;
        p00 = std::abs(p01 + p10);
        p01 = std::abs(p01);
        p10 = std::abs(p10);
        s8 p11a;
        if (p00 >= p01 && p10 >= p01)
            p11a = planes[i][pos + width];
        else if (p00 < p10)
            p11a = planes[i][pos];
        else
            p11a = planes[i][pos + 1];
        planes[i][pos + width + 1] += p11a + 0x80;
    }

    bstr output(width * height * 3);
    auto output_ptr = output.get<u8>();
    for (const auto y : algo::range(height))
    for (const auto x : algo::range(width))
    {
        const auto pos = y * width + x;
        s8 b = -128 + static_cast<s8>(planes[2][pos]);
        s8 r = -128 + static_cast<s8>(planes[1][pos]);
        s8 g = planes[0][pos] - ((b + r) >> 2);
        *output_ptr++ = r + g;
        *output_ptr++ = g;
        *output_ptr++ = b + g;
    }
    return output;
}

McgImageDecoder::McgImageDecoder()
{
    add_arg_parser_decorator(
        [](ArgParser &arg_parser)
        {
            arg_parser.register_switch({"--mcg-key"})
                ->set_value_name("KEY")
                ->set_description(
                    "Decryption key (0..255, same for all files)");
        },
        [&](const ArgParser &arg_parser)
        {
            if (arg_parser.has_switch("mcg-key"))
                key = algo::from_string<int>(arg_parser.get_switch("mcg-key"));
        });
}

bool McgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image McgImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    const auto version = static_cast<int>(0.5 + 100
        * algo::from_string<float>(input_file.stream.read(4).str()));

    input_file.stream.seek(16);
    const auto header_size = input_file.stream.read_le<u32>();
    if (header_size != 64)
    {
        throw err::NotSupportedError(
            algo::format("Unknown header size: %d", header_size));
    }
    const auto x = input_file.stream.read_le<u32>();
    const auto y = input_file.stream.read_le<u32>();
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    const auto depth = input_file.stream.read_le<u32>();
    const auto size_orig = input_file.stream.read_le<u32>();

    if (!key)
        throw err::UsageError("MCG decryption key not set");

    input_file.stream.seek(header_size);
    bstr data = input_file.stream.read_to_eof();
    if (version == 101)
        data = decrypt_v101(data, size_orig, key.get());
    else if (version == 200)
    {
        common::MrgDecryptor decryptor(data, width * height);
        bstr planes[3];
        for (const auto i : algo::range(3))
            planes[i] = decryptor.decrypt_with_key(key.get());
        data = transform_v200(planes, width, height);
    }
    else
        throw err::UnsupportedVersionError(version);
    data = common::fix_stride(data, width, height, depth);

    if (depth != 24)
        throw err::UnsupportedBitDepthError(depth);

    return res::Image(width, height, data, res::PixelFormat::BGR888);
}

static auto _ = dec::register_decoder<McgImageDecoder>("fc01/mcg");
