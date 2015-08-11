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

static const bstr magic = "CompressedBG___\x00"_b;

static u32 get_key(u32 *pkey)
{
    u32 key = *pkey;
    u32 tmp1 = 0x4E35 * (key & 0xFFFF);
    u32 tmp2 = 0x4E35 * (key >> 16);
    u32 tmp = 0x15A * key + tmp2 + (tmp1 >> 16);
    *pkey = (tmp << 16) + (tmp1 & 0xFFFF) + 1;
    return tmp & 0x7FFF;
}

static void decrypt(bstr &input, u32 key)
{
    for (auto i : util::range(input.size()))
        input.get<u8>()[i] -= static_cast<u8>(get_key(&key));
}

static u32 read_variable_data(u8 *&input, const u8 *input_guardian)
{
    u8 current;
    u32 result = 0;
    u32 shift = 0;
    do
    {
        assert(input < input_guardian);
        current = *input++;
        result |= (current & 0x7F) << shift;
        shift += 7;
    } while (current & 0x80);
    return result;
}

static void read_freq_table(io::IO &io, u32 raw_size, u32 key, u32 freq_table[])
{
    u8 expected_checksum1 = io.read_u8();
    u8 expected_checksum2 = io.read_u8();
    io.skip(2);

    bstr raw_data = io.read(raw_size);
    decrypt(raw_data, key);

    u8 checksum1 = 0, checksum2 = 0;
    for (auto i : util::range(raw_size))
    {
        checksum1 += raw_data.get<u8>()[i];
        checksum2 ^= raw_data.get<u8>()[i];
    }

    assert(checksum1 == expected_checksum1);
    assert(checksum2 == expected_checksum2);

    u8 *raw_data_ptr = raw_data.get<u8>();
    const u8 *raw_data_guardian = raw_data_ptr + raw_size;
    for (auto i : util::range(256))
        freq_table[i] = read_variable_data(raw_data_ptr, raw_data_guardian);
}

static int read_node_info(u32 freq_table[], NodeInfo node_info[])
{
    assert(freq_table);
    assert(node_info);
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
    bstr &huffman)
{
    assert(node_info);
    u32 root = last_node;

    for (auto i : util::range(huffman.size()))
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

static bstr decompress_rle(bstr &huffman, size_t output_size)
{
    u8 *huffman_ptr = huffman.get<u8>();
    const u8 *huffman_guardian = huffman_ptr + huffman.size();

    bstr output;
    output.resize(output_size);
    u8 *output_ptr = output.get<u8>();

    bool zero_flag = false;
    while (huffman_ptr < huffman_guardian)
    {
        u32 size = read_variable_data(huffman_ptr, huffman_guardian);
        if (zero_flag)
        {
            memset(output_ptr, 0, size);
            output_ptr += size;
        }
        else
        {
            memcpy(output_ptr, huffman_ptr, size);
            huffman_ptr += size;
            output_ptr += size;
        }
        zero_flag = !zero_flag;
    }

    return output;
}

static void transform_colors(bstr &input, u16 width, u16 height, u16 bpp)
{
    u16 channels = bpp >> 3;

    u8 *input_ptr = input.get<u8>();
    u8 *left = &input_ptr[- channels];
    u8 *above = &input_ptr[- width * channels];

    //ignore 0,0
    input_ptr += channels;
    above += channels;
    left += channels;

    //add left to first row
    for (auto x : util::range(1, width))
    {
        for (auto i : util::range(channels))
        {
            *input_ptr += input_ptr[-channels];
            input_ptr++;
            above++;
            left++;
        }
    }

    //add left and top to all other pixels
    for (auto y : util::range(1, height))
    {
        for (auto i : util::range(channels))
        {
            *input_ptr += *above;
            input_ptr++;
            above++;
            left++;
        }

        for (auto x : util::range(1, width))
        {
            for (auto i : util::range(channels))
            {
                *input_ptr += (*left  + *above) >> 1;
                input_ptr++;
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
    file.io.skip(magic.size());

    u16 width = file.io.read_u16_le();
    u16 height = file.io.read_u16_le();
    u16 bpp = file.io.read_u32_le();
    file.io.skip(8);

    u32 huffman_size = file.io.read_u32_le();
    u32 key = file.io.read_u32_le();
    u32 freq_table_data_size = file.io.read_u32_le();

    u32 freq_table[256];
    read_freq_table(file.io, freq_table_data_size, key, freq_table);

    NodeInfo node_info[511];
    int last_node = read_node_info(freq_table, node_info);

    io::BitReader bit_reader(file.io.read_to_eof());
    bstr huffman;
    huffman.resize(huffman_size);
    decompress_huffman(bit_reader, last_node, node_info, huffman);

    auto output = decompress_rle(huffman, width * height * (bpp >> 3));
    transform_colors(output, width, height, bpp);

    auto image = util::Image::from_pixels(
        width, height, output, bpp_to_image_pixel_format(bpp));
    return image->create_file(file.name);
}
