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

#include "dec/kirikiri/tlg/tlg5_decoder.h"
#include "algo/range.h"
#include "dec/kirikiri/tlg/lzss_decompressor.h"
#include "err.h"

using namespace au;
using namespace au::dec::kirikiri::tlg;

namespace
{
    struct Header final
    {
        u8 channel_count;
        u32 image_width;
        u32 image_height;
        u32 block_height;
    };

    struct BlockInfo final
    {
        BlockInfo(io::BaseByteStream &input_stream);
        void decompress(LzssDecompressor &decompressor, const Header &header);

        bool mark;
        size_t block_size;
        bstr data;
    };
}

BlockInfo::BlockInfo(io::BaseByteStream &input_stream)
{
    mark = input_stream.read<u8>() > 0;
    const auto block_size = input_stream.read_le<u32>();
    data = input_stream.read(block_size);
}

void BlockInfo::decompress(LzssDecompressor &decompressor, const Header &header)
{
    size_t output_size = header.image_width * header.block_height;
    data = decompressor.decompress(data, output_size);
}

static void load_pixel_block_row(
    res::Image &image,
    std::vector<std::unique_ptr<BlockInfo>> channel_data,
    const Header &header,
    int block_y)
{
    size_t max_y = std::min(block_y + header.block_height, header.image_height);
    bool use_alpha = header.channel_count == 4;

    for (const auto y : algo::range(block_y, max_y))
    {
        size_t block_y_shift = (y - block_y) * header.image_width;
        u8 prev_pixel[4] = {0, 0, 0, 0};

        for (const auto x : algo::range(header.image_width))
        {
            res::Pixel pixel;
            pixel.b = channel_data[0]->data.get<u8>()[block_y_shift + x];
            pixel.g = channel_data[1]->data.get<u8>()[block_y_shift + x];
            pixel.r = channel_data[2]->data.get<u8>()[block_y_shift + x];
            if (use_alpha)
                pixel.a = channel_data[3]->data.get<u8>()[block_y_shift + x];
            else
                pixel.a = 0xFF;
            pixel.b += pixel.g;
            pixel.r += pixel.g;

            for (const auto c : algo::range(header.channel_count))
            {
                prev_pixel[c] += pixel[c];
                image.at(x, y)[c] = prev_pixel[c];
                image.at(x, y)[c] += y > 0 ? image.at(x, y - 1)[c] : 0;
            }
            if (!use_alpha)
                image.at(x, y).a = 0xFF;
        }
    }
}

static void read_image(
    io::BaseByteStream &input_stream, res::Image &image, const Header &header)
{
    // ignore block sizes
    size_t block_count = (header.image_height - 1) / header.block_height + 1;
    input_stream.skip(4 * block_count);

    LzssDecompressor decompressor;
    for (const auto y
        : algo::range(0, header.image_height, header.block_height))
    {
        std::vector<std::unique_ptr<BlockInfo>> channel_data;
        for (const auto channel : algo::range(header.channel_count))
        {
            auto block_info = std::make_unique<BlockInfo>(input_stream);
            if (!block_info->mark)
                block_info->decompress(decompressor, header);
            channel_data.push_back(std::move(block_info));
        }
        load_pixel_block_row(image, std::move(channel_data), header, y);
    }
}

res::Image Tlg5Decoder::decode(io::File &file)
{
    Header header;
    header.channel_count = file.stream.read<u8>();
    header.image_width = file.stream.read_le<u32>();
    header.image_height = file.stream.read_le<u32>();
    header.block_height = file.stream.read_le<u32>();
    if (header.channel_count != 3 && header.channel_count != 4)
        throw err::UnsupportedChannelCountError(header.channel_count);

    res::Image image(header.image_width, header.image_height);
    read_image(file.stream, image, header);
    return image;
}
