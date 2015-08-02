// CBG image
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: -
// Archives:  ARC

#include <cassert>
#include <cstring>
#include "fmt/bgi/cbg_converter.h"
#include "io/bit_reader.h"
#include "io/buffered_io.h"
#include "util/range.h"
#include "util/image.h"

using namespace au;
using namespace au::fmt::bgi;

namespace
{
    struct NodeInfo
    {
        bool valid;
        u32 frequency;
        int left_node;
        int right_node;
    };
}

static const std::string magic = "CompressedBG___\x00"_s;

static u32 get_key(u32 *pkey)
{
    u32 key = *pkey;
    u32 tmp1 = 0x4E35 * (key & 0xFFFF);
    u32 tmp2 = 0x4E35 * (key >> 16);
    u32 tmp = 0x15A * key + tmp2 + (tmp1 >> 16);
    *pkey = (tmp << 16) + (tmp1 & 0xFFFF) + 1;
    return tmp & 0x7FFF;
}

static void decrypt(char *input, u32 decrypt_size, u32 key)
{
    for (auto i : util::range(decrypt_size))
        *input++ -= static_cast<char>(get_key(&key));
}

static u32 read_variable_data(char *&input, const char *input_guardian)
{
    char current;
    u32 result = 0;
    u32 shift = 0;
    do
    {
        current = *input++;
        assert(input <= input_guardian);
        result |= (current & 0x7F) << shift;
        shift += 7;
    } while (current & 0x80);
    return result;
}

static void read_freq_table(io::IO &io, u32 raw_size, u32 key, u32 freq_table[])
{
    std::unique_ptr<char[]> raw_data(new char[raw_size]);
    io.read(raw_data.get(), raw_size);

    decrypt(raw_data.get(), raw_size, key);

    char *raw_data_ptr = raw_data.get();
    const char *raw_data_guardian = raw_data_ptr + raw_size;
    for (auto i : util::range(256))
        freq_table[i] = read_variable_data(raw_data_ptr, raw_data_guardian);
}

static int read_node_info(u32 freq_table[], NodeInfo node_info[])
{
    assert(freq_table != nullptr);
    assert(node_info != nullptr);
    u32 frequency_sum = 0;
    for (auto i : util::range(256))
    {
        node_info[i].frequency = freq_table[i];
        node_info[i].valid = freq_table[i] > 0;
        node_info[i].left_node = i;
        node_info[i].right_node = i;
        frequency_sum += freq_table[i];
    }

    for (auto i : util::range(256, 511))
    {
        node_info[i].frequency = 0;
        node_info[i].valid = false;
        node_info[i].left_node = -1;
        node_info[i].right_node = -1;
    }

    for (auto i : util::range(256, 511))
    {
        u32 frequency = 0;
        int children[2];
        for (auto j : util::range(2))
        {
            u32 min = 0xFFFFFFFF;
            children[j] = -1;
            for (auto k : util::range(i))
            {
                if (node_info[k].valid && node_info[k].frequency < min)
                {
                    min = node_info[k].frequency;
                    children[j] = k;
                }
            }
            if (children[j] != -1)
            {
                node_info[children[j]].valid = false;
                frequency += node_info[children[j]].frequency;
            }
        }
        node_info[i].valid = true;
        node_info[i].frequency = frequency;
        node_info[i].left_node = children[0];
        node_info[i].right_node = children[1];
        if (frequency == frequency_sum)
            return i;
    }
    return 511;
}

static void decompress_huffman(
    io::BitReader &bit_reader,
    int last_node,
    NodeInfo node_info[],
    u32 huffman_size,
    u8 *huffman)
{
    assert(node_info != nullptr);
    u32 root = last_node;

    for (auto i : util::range(huffman_size))
    {
        int node = root;
        while (node >= 256)
        {
            node = bit_reader.get(1)
                ? node_info[node].right_node
                : node_info[node].left_node;
        }
        huffman[i] = node;
    }
}

static void decompress_rle(u32 huffman_size, char *huffman, char *output)
{
    assert(huffman != nullptr);
    assert(output != nullptr);
    char *huffman_ptr = huffman;
    const char *huffman_guardian = huffman + huffman_size;
    bool zero_flag = false;
    while (huffman_ptr < huffman_guardian)
    {
        u32 length = read_variable_data(huffman_ptr, huffman_guardian);
        if (zero_flag)
        {
            memset(output, 0, length);
            output += length;
        }
        else
        {
            memcpy(output, huffman_ptr, length);
            huffman_ptr += length;
            output += length;
        }
        zero_flag = !zero_flag;
    }
}

static void transform_colors(u8 *input, u16 width, u16 height, u16 bpp)
{
    assert(input != nullptr);
    u16 channels = bpp >> 3;

    u8 *left = &input[- channels];
    u8 *above = &input[- width * channels];

    //ignore 0,0
    input += channels;
    above += channels;
    left += channels;

    //add left to first row
    for (auto x : util::range(1, width))
    {
        for (auto i : util::range(channels))
        {
            *input += input[-channels];
            input++;
            above++;
            left++;
        }
    }

    //add left and top to all other pixels
    for (auto y : util::range(1, height))
    {
        for (auto i : util::range(channels))
        {
            *input += *above;
            input++;
            above++;
            left++;
        }

        for (auto x : util::range(1, width))
        {
            for (auto i : util::range(channels))
            {
                *input += (*left  + *above) >> 1;
                input++;
                above++;
                left++;
            }
        }
    }
}

static util::PixelFormat bpp_to_image_pixel_format(int bpp)
{
    switch (bpp)
    {
        case 8:
            return util::PixelFormat::Grayscale;
        case 24:
            return util::PixelFormat::BGR;
        case 32:
            return util::PixelFormat::BGRA;
    }
    throw std::runtime_error("Unsupported BPP");
}

bool CbgConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> CbgConverter::decode_internal(File &file) const
{
    file.io.seek(0);
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a CBG image");

    u16 width = file.io.read_u16_le();
    u16 height = file.io.read_u16_le();
    u16 bpp = file.io.read_u16_le();
    file.io.skip(10);

    u32 huffman_size = file.io.read_u32_le();
    u32 key = file.io.read_u32_le();
    u32 freq_table_data_size = file.io.read_u32_le();
    file.io.skip(4);

    u32 freq_table[256];
    read_freq_table(file.io, freq_table_data_size, key, freq_table);

    NodeInfo node_info[511];
    int last_node = read_node_info(freq_table, node_info);

    std::unique_ptr<char[]> huffman(new char[huffman_size]);

    io::BufferedIO buffered_io(file.io);
    io::BitReader bit_reader(buffered_io);
    decompress_huffman(
        bit_reader,
        last_node,
        node_info,
        huffman_size,
        reinterpret_cast<u8*>(huffman.get()));

    size_t output_size = width * height * (bpp >> 3);
    std::unique_ptr<char[]> output(new char[output_size]);
    decompress_rle(huffman_size, huffman.get(), output.get());
    transform_colors(reinterpret_cast<u8*>(output.get()), width, height, bpp);

    auto image = util::Image::from_pixels(
        width,
        height,
        std::string(output.get(), output_size),
        bpp_to_image_pixel_format(bpp));
    return image->create_file(file.name);
}
