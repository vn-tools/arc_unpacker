#include <cstring>
#include <vector>
#include "formats/gfx/tlg_converter/lzss_decompressor.h"
#include "formats/gfx/tlg_converter/tlg5_decoder.h"
#include "formats/image.h"

namespace
{
    typedef unsigned char uchar;

    typedef struct
    {
        uint8_t channel_count;
        uint32_t image_width;
        uint32_t image_height;
        uint32_t block_height;
    } Header;

    typedef struct BlockInfo
    {
        bool mark;
        size_t block_size;
        std::unique_ptr<uchar[]> block_data;

        BlockInfo(IO &io) : block_data(nullptr)
        {
            mark = io.read_u8() > 0;
            block_size = io.read_u32_le();
            block_data.reset(new uchar[block_size]);
            uchar *tmp = block_data.get();
            for (size_t i = 0; i < block_size; i ++)
                *tmp ++ = io.read_u8();
        }

        void decompress(
            LzssDecompressor &decompressor,
            Header &header)
        {
            size_t new_data_size = header.image_width * header.block_height;
            std::unique_ptr<uchar[]> new_data(new uchar[new_data_size]);

            decompressor.decompress(
                block_data.get(), block_size, new_data.get(), new_data_size);

            block_data = std::move(new_data);
            block_size = new_data_size;
        }


    } BlockInfo;

    void load_pixel_block_row(
        uchar *zero_line,
        uchar *output,
        std::vector<std::unique_ptr<BlockInfo>> channel_data,
        Header &header,
        int block_y)
    {
        size_t max_y = block_y + header.block_height;
        if (max_y > header.image_height)
            max_y = header.image_height;
        bool use_alpha = header.channel_count == 4;

        uchar *current_line = &output[block_y * header.image_width * 4];
        uchar *previous_line = block_y == 0
            ? zero_line
            : &output[(block_y - 1) * header.image_width * 4];

        for (size_t y = block_y; y < max_y; y ++)
        {
            uchar prev_r = 0;
            uchar prev_g = 0;
            uchar prev_b = 0;
            uchar prev_a = 0;

            size_t block_y_shift = (y - block_y) * header.image_width;
            uchar *current_line_start = current_line;
            for (size_t x = 0; x < header.image_width; x ++)
            {
                uchar r = channel_data[2]->block_data[block_y_shift + x];
                uchar g = channel_data[1]->block_data[block_y_shift + x];
                uchar b = channel_data[0]->block_data[block_y_shift + x];
                uchar a = use_alpha
                    ? channel_data[3]->block_data[block_y_shift + x]
                    : 0;

                r += g;
                b += g;

                prev_r += r;
                prev_g += g;
                prev_b += b;
                prev_a += a;

                uchar output_r = prev_r;
                uchar output_g = prev_g;
                uchar output_b = prev_b;
                uchar output_a = prev_a;

                output_r += *previous_line ++;
                output_g += *previous_line ++;
                output_b += *previous_line ++;
                output_a += *previous_line ++;

                if (!use_alpha)
                    output_a = 0xff;

                *current_line ++ = output_r;
                *current_line ++ = output_g;
                *current_line ++ = output_b;
                *current_line ++ = output_a;
            }
            previous_line = current_line_start;
        }
    }

    void read_pixels(
        IO &io,
        uchar *output,
        Header &header)
    {
        // ignore block sizes
        size_t block_count
            = (header.image_height - 1) / header.block_height + 1;
        io.skip(4 * block_count);

        LzssDecompressor decompressor;

        std::unique_ptr<uchar> zero_line(new uchar[header.image_width * 4]);
        memset(zero_line.get(), 0, header.image_width * 4);
        memset(output, 0, header.image_width * header.image_height * 4);

        for (size_t y = 0; y < header.image_height; y += header.block_height)
        {
            std::vector<std::unique_ptr<BlockInfo>> channel_data;

            for (size_t channel = 0; channel < header.channel_count; channel ++)
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
}

void Tlg5Decoder::decode(VirtualFile &file)
{
    Header header;
    header.channel_count = file.io.read_u8();
    header.image_width = file.io.read_u32_le();
    header.image_height = file.io.read_u32_le();
    header.block_height = file.io.read_u32_le();
    if (header.channel_count != 3 && header.channel_count != 4)
        throw std::runtime_error("Unsupported channel count");

    size_t pixels_size = header.image_width * header.image_height * 4;
    std::unique_ptr<unsigned char> pixels(new unsigned char[pixels_size]);

    read_pixels(file.io, pixels.get(), header);

    std::unique_ptr<Image> image = Image::from_pixels(
        header.image_width,
        header.image_height,
        std::string(reinterpret_cast<char*>(pixels.get()), pixels_size),
        IMAGE_PIXEL_FORMAT_RGBA);
    image->update_file(file);
}
