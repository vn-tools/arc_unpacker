#include <cstring>
#include <stdexcept>
#include <vector>
#include "fmt/kirikiri/tlg/lzss_decompressor.h"
#include "fmt/kirikiri/tlg/tlg5_decoder.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::kirikiri::tlg;

namespace
{
    struct Header
    {
        u8 channel_count;
        u32 image_width;
        u32 image_height;
        u32 block_height;
    };

    struct BlockInfo
    {
        bool mark;
        size_t block_size;
        bstr data;
        BlockInfo(io::IO &io);
        void decompress(LzssDecompressor &decompressor, Header &header);
    };
}

BlockInfo::BlockInfo(io::IO &io)
{
    mark = io.read_u8() > 0;
    data = io.read(io.read_u32_le());
}

void BlockInfo::decompress(LzssDecompressor &decompressor, Header &header)
{
    size_t output_size = header.image_width * header.block_height;
    data = decompressor.decompress(data, output_size);
}

static void load_pixel_block_row(
    bstr &zero_line,
    bstr &output,
    std::vector<std::unique_ptr<BlockInfo>> channel_data,
    Header &header,
    int block_y)
{
    size_t max_y = block_y + header.block_height;
    if (max_y > header.image_height)
        max_y = header.image_height;
    bool use_alpha = header.channel_count == 4;

    u8 *current_line = output.get<u8>() + block_y * header.image_width * 4;
    u8 *previous_line = block_y == 0
        ? zero_line.get<u8>()
        : output.get<u8>() + (block_y - 1) * header.image_width * 4;

    for (auto y : util::range(block_y, max_y))
    {
        size_t block_y_shift = (y - block_y) * header.image_width;
        u8 *current_line_start = current_line;
        u8 prev_rgba[4] = { 0, 0, 0, 0 };

        for (auto x : util::range(header.image_width))
        {
            u8 rgba[4] =
            {
                channel_data[2]->data.get<u8>(block_y_shift + x),
                channel_data[1]->data.get<u8>(block_y_shift + x),
                channel_data[0]->data.get<u8>(block_y_shift + x),
                use_alpha
                    ? channel_data[3]->data.get<u8>(block_y_shift + x)
                    : static_cast<u8>(0)
            };
            rgba[0] += rgba[1];
            rgba[2] += rgba[1];

            u8 output_rgba[4];
            for (auto c : util::range(4))
            {
                prev_rgba[c] += rgba[c];
                output_rgba[c] = prev_rgba[c];
                output_rgba[c] += *previous_line++;
            }
            if (!use_alpha)
                output_rgba[3] = 0xFF;

            for (auto c : util::range(4))
                *current_line++ = output_rgba[c];
        }
        previous_line = current_line_start;
    }
}

static void read_pixels(io::IO &io, bstr &output, Header &header)
{
    // ignore block sizes
    size_t block_count = (header.image_height - 1) / header.block_height + 1;
    io.skip(4 * block_count);

    LzssDecompressor decompressor;

    bstr zero_line;
    zero_line.resize(header.image_width * 4);

    for (auto y : util::range(0, header.image_height, header.block_height))
    {
        std::vector<std::unique_ptr<BlockInfo>> channel_data;

        for (auto channel : util::range(header.channel_count))
        {
            std::unique_ptr<BlockInfo> block_info(new BlockInfo(io));
            if (!block_info->mark)
                block_info->decompress(decompressor, header);
            channel_data.push_back(std::move(block_info));
        }

        load_pixel_block_row(
            zero_line, output, std::move(channel_data), header, y);
    }
}

std::unique_ptr<File> Tlg5Decoder::decode(File &file)
{
    Header header;
    header.channel_count = file.io.read_u8();
    header.image_width = file.io.read_u32_le();
    header.image_height = file.io.read_u32_le();
    header.block_height = file.io.read_u32_le();
    if (header.channel_count != 3 && header.channel_count != 4)
        throw std::runtime_error("Unsupported channel count");

    bstr pixels;
    pixels.resize(header.image_width * header.image_height * 4);
    read_pixels(file.io, pixels, header);

    std::unique_ptr<util::Image> image = util::Image::from_pixels(
        header.image_width,
        header.image_height,
        pixels,
        util::PixelFormat::RGBA);
    return image->create_file(file.name);
}
