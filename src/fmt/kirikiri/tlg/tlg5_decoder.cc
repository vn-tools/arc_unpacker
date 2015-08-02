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
        std::unique_ptr<u8[]> block_data;
        BlockInfo(io::IO &io);
        void decompress(LzssDecompressor &decompressor, Header &header);
    };
}

BlockInfo::BlockInfo(io::IO &io) : block_data(nullptr)
{
    mark = io.read_u8() > 0;
    block_size = io.read_u32_le();
    block_data.reset(new u8[block_size]);
    u8 *tmp = block_data.get();
    for (auto i : util::range(block_size))
        *tmp++ = io.read_u8();
}

void BlockInfo::decompress(LzssDecompressor &decompressor, Header &header)
{
    size_t new_data_size = header.image_width * header.block_height;
    std::unique_ptr<u8[]> new_data(new u8[new_data_size]);

    decompressor.decompress(
        block_data.get(), block_size, new_data.get(), new_data_size);

    block_data = std::move(new_data);
    block_size = new_data_size;
}

static void load_pixel_block_row(
    u8 *zero_line,
    u8 *output,
    std::vector<std::unique_ptr<BlockInfo>> channel_data,
    Header &header,
    int block_y)
{
    size_t max_y = block_y + header.block_height;
    if (max_y > header.image_height)
        max_y = header.image_height;
    bool use_alpha = header.channel_count == 4;

    u8 *current_line = &output[block_y * header.image_width * 4];
    u8 *previous_line = block_y == 0
        ? zero_line
        : &output[(block_y - 1) * header.image_width * 4];

    for (auto y : util::range(block_y, max_y))
    {
        u8 prev_r = 0;
        u8 prev_g = 0;
        u8 prev_b = 0;
        u8 prev_a = 0;

        size_t block_y_shift = (y - block_y) * header.image_width;
        u8 *current_line_start = current_line;
        for (auto x : util::range(header.image_width))
        {
            u8 r = channel_data[2]->block_data[block_y_shift + x];
            u8 g = channel_data[1]->block_data[block_y_shift + x];
            u8 b = channel_data[0]->block_data[block_y_shift + x];
            u8 a = use_alpha
                ? channel_data[3]->block_data[block_y_shift + x]
                : 0;

            r += g;
            b += g;

            prev_r += r;
            prev_g += g;
            prev_b += b;
            prev_a += a;

            u8 output_r = prev_r;
            u8 output_g = prev_g;
            u8 output_b = prev_b;
            u8 output_a = prev_a;

            output_r += *previous_line++;
            output_g += *previous_line++;
            output_b += *previous_line++;
            output_a += *previous_line++;

            if (!use_alpha)
                output_a = 0xFF;

            *current_line++ = output_r;
            *current_line++ = output_g;
            *current_line++ = output_b;
            *current_line++ = output_a;
        }
        previous_line = current_line_start;
    }
}

static void read_pixels(io::IO &io, u8 *output, Header &header)
{
    // ignore block sizes
    size_t block_count = (header.image_height - 1) / header.block_height + 1;
    io.skip(4 * block_count);

    LzssDecompressor decompressor;

    std::unique_ptr<u8[]> zero_line(new u8[header.image_width * 4]);
    memset(zero_line.get(), 0, header.image_width * 4);
    memset(output, 0, header.image_width * header.image_height * 4);

    for (auto y : util::range(0, header.image_height, header.block_height))
    {
        std::vector<std::unique_ptr<BlockInfo>> channel_data;

        for (size_t channel = 0; channel < header.channel_count; channel++)
        {
            std::unique_ptr<BlockInfo> block_info(new BlockInfo(io));
            if (!block_info->mark)
                block_info->decompress(decompressor, header);
            channel_data.push_back(std::move(block_info));
        }

        load_pixel_block_row(
            zero_line.get(), output, std::move(channel_data), header, y);
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

    size_t pixels_size = header.image_width * header.image_height * 4;
    std::unique_ptr<u8[]> pixels(new u8[pixels_size]);

    read_pixels(file.io, pixels.get(), header);

    std::unique_ptr<util::Image> image = util::Image::from_pixels(
        header.image_width,
        header.image_height,
        std::string(reinterpret_cast<char*>(pixels.get()), pixels_size),
        util::PixelFormat::RGBA);
    return image->create_file(file.name);
}
