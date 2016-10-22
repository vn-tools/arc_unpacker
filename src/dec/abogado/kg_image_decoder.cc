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

#include "dec/abogado/kg_image_decoder.h"
#include <array>
#include "algo/ptr.h"
#include "err.h"
#include "io/msb_bit_stream.h"

using namespace au;
using namespace au::dec::abogado;

static const auto magic = "KG\x02"_b;

namespace
{
    using Table = std::array<std::array<u8, 8>, 256>;
}

static Table create_table()
{
    Table table;
    for (const auto i : algo::range(256))
    for (const auto j : algo::range(8))
        table[i][j] = j;
    return table;
}

static int read_integer(io::MsbBitStream &input_stream)
{
    int tmp;

    tmp = input_stream.read(2);
    if (tmp)
        return tmp;

    tmp = input_stream.read(4);
    if (tmp)
        return tmp + 3;

    tmp = input_stream.read(8);
    if (tmp)
        return tmp;

    tmp = input_stream.read(16);
    if (tmp)
        return tmp;

    tmp = input_stream.read(16) << 16;
    return input_stream.read(16) + tmp;
}

static int choose_strategy(io::MsbBitStream &input_stream)
{
    if (input_stream.read(1))
    {
        if (input_stream.read(1))
            return 0xC + input_stream.read(2);
        return 2;
    }
    return 0;
}

static bstr read_single_channel(
    io::MsbBitStream &input_stream,
    Table &table,
    const size_t width,
    const size_t height)
{
    bstr data(width * height);
    auto data_ptr = algo::make_ptr(data);

    for (const auto i : algo::range(2))
        data_ptr.append_basic(static_cast<u8>(input_stream.read(8)));

    while (data_ptr.left())
    {
        const auto strategy = choose_strategy(input_stream);
        switch (strategy)
        {
            case 0:
            {
                const auto last_byte = data_ptr[-1];
                auto &chunk = table[last_byte];

                const auto value = input_stream.read(1)
                    ? input_stream.read(8)
                    : chunk[input_stream.read(3)];

                const auto it = std::find(
                    std::begin(chunk),
                    std::end(chunk),
                    value);
                const auto pos = it == std::end(chunk)
                    ? 7
                    : std::distance(std::begin(chunk), it);

                for (const auto i : algo::range(pos-1,-1,-1))
                    chunk[i + 1] = chunk[i];
                chunk[0] = value;

                data_ptr.append_basic(value);
                break;
            }

            case 2:
                data_ptr.append_self(-1, read_integer(input_stream));
                break;

            case 0xC:
                data_ptr.append_self(-width, read_integer(input_stream));
                break;

            case 0xD:
                data_ptr.append_self(-width + 1, read_integer(input_stream));
                break;

            case 0xE:
                data_ptr.append_self(-width - 1, read_integer(input_stream));
                break;

            case 0xF:
                data_ptr.append_self(-2, read_integer(input_stream));
                break;
        }
    }

    return data;
}

bool KgImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image KgImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto type = input_file.stream.read<u8>();
    const auto width = input_file.stream.read_le<u16>();
    const auto height = input_file.stream.read_le<u16>();
    input_file.stream.skip(4);
    const auto palette_offset = input_file.stream.read_le<u32>();
    const auto data_offset = input_file.stream.read_le<u32>();
    const auto file_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(20);
    const auto alpha_offset = input_file.stream.read_le<u32>();

    std::unique_ptr<res::Image> image;

    if (type == 1)
    {
        res::Palette palette(
            256,
            input_file.stream.seek(palette_offset),
            res::PixelFormat::BGR888X);
        input_file.stream.seek(data_offset);
        io::MsbBitStream bit_stream(input_file.stream);
        auto table = create_table();
        const auto data = read_single_channel(bit_stream, table, width, height);
        image = std::make_unique<res::Image>(width, height, data, palette);
    }

    if (type == 2)
    {
        input_file.stream.seek(data_offset);
        io::MsbBitStream bit_stream(input_file.stream);
        auto table = create_table();
        const bstr channels[3] =
        {
            read_single_channel(bit_stream, table, width, height),
            read_single_channel(bit_stream, table, width, height),
            read_single_channel(bit_stream, table, width, height),
        };
        bstr data(width * height * 3);
        for (const auto i : algo::range(width * height))
        for (const auto j : algo::range(3))
            data[i * 3 + j] = channels[j][i];
        image = std::make_unique<res::Image>(
            width, height, data, res::PixelFormat::BGR888);
    }

    if (!image)
        throw err::UnsupportedVersionError(type);

    if (alpha_offset)
    {
        try
        {
            input_file.stream.seek(alpha_offset);
            io::MsbBitStream bit_stream(input_file.stream);
            auto table = create_table();
            const auto data = read_single_channel(
                bit_stream, table, width, height);
            res::Image mask(width, height, data, res::PixelFormat::Gray8);
            image->apply_mask(mask);
        }
        catch (const err::EofError)
        {
            // PCLICK and FRAME for "Pigeon Blood" seem to contain garbage here
            // (the game hardcodes decoding of these files and simply doesn't
            // call the function for decoding the alpha channel for them)
        }
    }

    image->flip_vertically();
    return *image;
}

static auto _
    = dec::register_decoder<KgImageDecoder>("abogado/kg");
