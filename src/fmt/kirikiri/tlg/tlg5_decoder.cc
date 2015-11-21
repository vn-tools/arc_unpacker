#include "fmt/kirikiri/tlg/tlg5_decoder.h"
#include <algorithm>
#include "err.h"
#include "fmt/kirikiri/tlg/lzss_decompressor.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::kirikiri::tlg;

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
        BlockInfo(io::Stream &stream);
        void decompress(LzssDecompressor &decompressor, Header &header);

        bool mark;
        size_t block_size;
        bstr data;
    };
}

BlockInfo::BlockInfo(io::Stream &stream)
{
    mark = stream.read_u8() > 0;
    data = stream.read(stream.read_u32_le());
}

void BlockInfo::decompress(LzssDecompressor &decompressor, Header &header)
{
    size_t output_size = header.image_width * header.block_height;
    data = decompressor.decompress(data, output_size);
}

static void load_pixel_block_row(
    pix::Grid &pixels,
    std::vector<std::unique_ptr<BlockInfo>> channel_data,
    Header &header,
    int block_y)
{
    size_t max_y = std::min(block_y + header.block_height, header.image_height);
    bool use_alpha = header.channel_count == 4;

    for (auto y : util::range(block_y, max_y))
    {
        size_t block_y_shift = (y - block_y) * header.image_width;
        u8 prev_pixel[4] = {0, 0, 0, 0};

        for (auto x : util::range(header.image_width))
        {
            pix::Pixel pixel;
            pixel.b = channel_data[0]->data.get<u8>()[block_y_shift + x];
            pixel.g = channel_data[1]->data.get<u8>()[block_y_shift + x];
            pixel.r = channel_data[2]->data.get<u8>()[block_y_shift + x];
            if (use_alpha)
                pixel.a = channel_data[3]->data.get<u8>()[block_y_shift + x];
            pixel.b += pixel.g;
            pixel.r += pixel.g;

            for (auto c : util::range(header.channel_count))
            {
                prev_pixel[c] += pixel[c];
                pixels.at(x, y)[c] = prev_pixel[c];
                pixels.at(x, y)[c] += y > 0 ? pixels.at(x, y - 1)[c] : 0;
            }
            if (!use_alpha)
                pixels.at(x, y).a = 0xFF;
        }
    }
}

static void read_pixels(io::Stream &stream, pix::Grid &pixels, Header &header)
{
    // ignore block sizes
    size_t block_count = (header.image_height - 1) / header.block_height + 1;
    stream.skip(4 * block_count);

    LzssDecompressor decompressor;

    for (auto y : util::range(0, header.image_height, header.block_height))
    {
        std::vector<std::unique_ptr<BlockInfo>> channel_data;

        for (auto channel : util::range(header.channel_count))
        {
            auto block_info = std::make_unique<BlockInfo>(stream);
            if (!block_info->mark)
                block_info->decompress(decompressor, header);
            channel_data.push_back(std::move(block_info));
        }

        load_pixel_block_row(pixels, std::move(channel_data), header, y);
    }
}

pix::Grid Tlg5Decoder::decode(io::File &file)
{
    Header header;
    header.channel_count = file.stream.read_u8();
    header.image_width = file.stream.read_u32_le();
    header.image_height = file.stream.read_u32_le();
    header.block_height = file.stream.read_u32_le();
    if (header.channel_count != 3 && header.channel_count != 4)
        throw err::UnsupportedChannelCountError(header.channel_count);

    pix::Grid pixels(header.image_width, header.image_height);
    read_pixels(file.stream, pixels, header);
    return pixels;
}
