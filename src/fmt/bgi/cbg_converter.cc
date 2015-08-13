// CBG image
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: -
// Archives:  ARC
//
// Known games:
// - Higurashi No Naku Koro Ni
// - Go! Go! Nippon! ~My First Trip to Japan~

#include <cstring>
#include <array>
#include "fmt/bgi/cbg_converter.h"
#include "fmt/bgi/common.h"
#include "io/bit_reader.h"
#include "util/image.h"
#include "util/range.h"
#include "util/require.h"

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

    using NodeList = std::vector<std::unique_ptr<NodeInfo>>;
    using FreqTable = std::array<u32, 256>;
}

static const bstr magic = "CompressedBG___\x00"_b;

static bstr get_decrypted_data(io::IO &io)
{
    u32 key = io.read_u32_le();
    u32 data_size = io.read_u32_le();

    u8 expected_checksum1 = io.read_u8();
    u8 expected_checksum2 = io.read_u8();
    io.skip(2);

    bstr data = io.read(data_size);
    u8 *data_ptr = data.get<u8>();

    u8 checksum1 = 0, checksum2 = 0;
    for (auto i : util::range(data.size()))
    {
        *data_ptr -= get_and_update_key(key);
        checksum1 += *data_ptr;
        checksum2 ^= *data_ptr;
        data_ptr++;
    }

    util::require(checksum1 == expected_checksum1);
    util::require(checksum2 == expected_checksum2);
    return data;
}

static u32 read_variable_data(const u8 *&input, const u8 *input_guardian)
{
    u8 current;
    u32 result = 0;
    u32 shift = 0;
    do
    {
        util::require(input < input_guardian);
        current = *input++;
        result |= (current & 0x7F) << shift;
        shift += 7;
    } while (current & 0x80);
    return result;
}

static FreqTable read_freq_table(const bstr &raw_data)
{
    FreqTable freq_table;
    const u8 *raw_data_ptr = raw_data.get<const u8>();
    const u8 *raw_data_guardian = raw_data_ptr + raw_data.size();
    for (auto i : util::range(256))
        freq_table[i] = read_variable_data(raw_data_ptr, raw_data_guardian);
    return freq_table;
}

static NodeList read_nodes(const FreqTable &freq_table)
{
    NodeList nodes;

    u32 frequency_sum = 0;
    for (auto i : util::range(256))
    {
        std::unique_ptr<NodeInfo> node(new NodeInfo);
        node->frequency = freq_table[i];
        node->valid = freq_table[i] > 0;
        node->left_node = i;
        node->right_node = i;
        frequency_sum += freq_table[i];
        nodes.push_back(std::move(node));
    }

    for (auto i : util::range(256, 512))
    {
        u32 frequency = 0;
        int children[2];
        for (auto j : util::range(2))
        {
            u32 min = 0xFFFFFFFF;
            children[j] = -1;
            for (auto k : util::range(i))
            {
                if (nodes[k]->valid && nodes[k]->frequency < min)
                {
                    min = nodes[k]->frequency;
                    children[j] = k;
                }
            }
            if (children[j] != -1)
            {
                nodes[children[j]]->valid = false;
                frequency += nodes[children[j]]->frequency;
            }
        }

        std::unique_ptr<NodeInfo> node(new NodeInfo);
        node->valid = true;
        node->frequency = frequency;
        node->left_node = children[0];
        node->right_node = children[1];
        nodes.push_back(std::move(node));

        if (frequency == frequency_sum)
            break;
    }
    return nodes;
}

static bstr decompress_huffman(
    io::BitReader &bit_reader, const NodeList &nodes, size_t huffman_size)
{
    bstr huffman;
    huffman.resize(huffman_size);
    u32 root = nodes.size() - 1;
    for (auto i : util::range(huffman.size()))
    {
        size_t node = root;
        while (node >= 256)
        {
            node = bit_reader.get(1)
                ? nodes[node]->right_node
                : nodes[node]->left_node;
        }
        huffman[i] = node;
    }
    return huffman;
}

static bstr decompress_rle(bstr &huffman, size_t output_size)
{
    const u8 *huffman_ptr = huffman.get<const u8>();
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
    bstr decrypted_data = get_decrypted_data(file.io);

    auto freq_table = read_freq_table(decrypted_data);
    auto nodes = read_nodes(freq_table);

    io::BitReader bit_reader(file.io.read_to_eof());
    bstr huffman = decompress_huffman(bit_reader, nodes, huffman_size);

    auto output = decompress_rle(huffman, width * height * (bpp >> 3));
    transform_colors(output, width, height, bpp);

    auto image = util::Image::from_pixels(
        width, height, output, bpp_to_image_pixel_format(bpp));
    return image->create_file(file.name);
}
