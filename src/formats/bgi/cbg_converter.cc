// CBG image
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: -
// Archives:  ARC

#include <cassert>
#include <cstring>
#include "formats/bgi/cbg_converter.h"
#include "io/bit_reader.h"
#include "io/buffered_io.h"
#include "util/image.h"
using namespace Formats::Bgi;

namespace
{
    const std::string bmp_magic("BM6", 3);
    const std::string magic("CompressedBG___\x00", 16);

    typedef struct
    {
        bool valid;
        uint32_t frequency;
        int left_node;
        int right_node;
    } NodeInfo;

    uint32_t get_key(uint32_t *pkey)
    {
        uint32_t key = *pkey;
        uint32_t tmp1 = 0x4e35 * (key & 0xffff);
        uint32_t tmp2 = 0x4e35 * (key >> 16);
        uint32_t tmp = 0x15a * key + tmp2 + (tmp1 >> 16);
        *pkey = (tmp << 16) + (tmp1 & 0xffff) + 1;
        return tmp & 0x7fff;
    }

    void decrypt(char *input, uint32_t decrypt_size, uint32_t key)
    {
        uint32_t i;
        for (i = 0; i < decrypt_size; i ++)
        {
            *input ++ -= (char) get_key(&key);
        }
    }

    uint32_t read_variable_data(char *&input, const char *input_guardian)
    {
        char current;
        uint32_t result = 0;
        uint32_t shift = 0;
        do
        {
            current = *input ++;
            assert(input <= input_guardian);
            result |= (current & 0x7f) << shift;
            shift += 7;
        } while (current & 0x80);
        return result;
    }

    void read_frequency_table(
        IO &io,
        uint32_t raw_data_size,
        uint32_t key,
        uint32_t frequency_table[])
    {
        std::unique_ptr<char[]> raw_data(new char[raw_data_size]);
        io.read(raw_data.get(), raw_data_size);

        decrypt(raw_data.get(), raw_data_size, key);

        int i;
        char *raw_data_ptr = raw_data.get();
        const char *raw_data_guardian = raw_data_ptr + raw_data_size;
        for (i = 0; i < 256; i ++)
        {
            frequency_table[i] = read_variable_data(
                raw_data_ptr,
                raw_data_guardian);
        }
    }

    int read_node_info(uint32_t frequency_table[], NodeInfo node_info[])
    {
        assert(frequency_table != nullptr);
        assert(node_info != nullptr);
        int i, j, k;
        uint32_t frequency_sum = 0;
        for (i = 0; i < 256; i ++)
        {
            node_info[i].frequency = frequency_table[i];
            node_info[i].valid = frequency_table[i] > 0;
            node_info[i].left_node = i;
            node_info[i].right_node = i;
            frequency_sum += frequency_table[i];
        }

        for (i = 256; i < 511; i ++)
        {
            node_info[i].frequency = 0;
            node_info[i].valid = false;
            node_info[i].left_node = -1;
            node_info[i].right_node = -1;
        }

        for (i = 256; i < 511; i ++)
        {
            uint32_t frequency = 0;
            int children[2];
            for (j = 0; j < 2; j ++)
            {
                uint32_t min = 0xffffffff;
                children[j] = -1;
                for (k = 0; k < i; k ++)
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
                break;
        }
        return i;
    }

    void decompress_huffman(
        BitReader &bit_reader,
        int last_node,
        NodeInfo node_info[],
        uint32_t huffman_size,
        uint8_t *huffman)
    {
        assert(node_info != nullptr);
        uint32_t root = last_node;

        for (size_t i = 0; i < huffman_size; i ++)
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

    void decompress_rle(
        uint32_t huffman_size,
        char *huffman,
        char *output)
    {
        assert(huffman != nullptr);
        assert(output != nullptr);
        char *huffman_ptr = huffman;
        const char *huffman_guardian = huffman + huffman_size;
        bool zero_flag = false;
        while (huffman_ptr < huffman_guardian)
        {
            uint32_t length = read_variable_data(huffman_ptr, huffman_guardian);
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

    void transform_colors(
        uint8_t *input,
        uint16_t width,
        uint16_t height,
        uint16_t bpp)
    {
        assert(input != nullptr);
        uint16_t channels = bpp >> 3;
        int y, x, i;

        uint8_t *left = &input[- channels];
        uint8_t *above = &input[- width * channels];

        //ignore 0,0
        input += channels;
        above += channels;
        left += channels;

        //add left to first row
        for (x = 1; x < width; x ++)
        {
            for (i = 0; i < channels; i ++)
            {
                *input += input[-channels];
                input ++;
                above ++;
                left ++;
            }
        }

        //add left and top to all other pixels
        for (y = 1; y < height; y ++)
        {
            for (i = 0; i < channels; i ++)
            {
                *input += *above;
                input ++;
                above ++;
                left ++;
            }

            for (x = 1; x < width; x ++)
            {
                for (i = 0; i < channels; i ++)
                {
                    *input += (*left  + *above) >> 1;
                    input ++;
                    above ++;
                    left ++;
                }
            }
        }
    }

    PixelFormat bpp_to_image_pixel_format(short bpp)
    {
        switch (bpp)
        {
            case 8:
                return PixelFormat::Grayscale;
            case 24:
                return PixelFormat::BGR;
            case 32:
                return PixelFormat::BGRA;
        }
        throw std::runtime_error("Unsupported BPP");
    }
}

bool CbgConverter::is_recognized_internal(File &file) const
{
    if (file.io.read(bmp_magic.size()) == bmp_magic)
        return true;
    file.io.seek(0);
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> CbgConverter::decode_internal(File &file) const
{
    if (file.io.read(bmp_magic.size()) == bmp_magic)
    {
        file.io.seek(0);
        std::unique_ptr<File> output_file(new File);
        output_file->io.write_from_io(file.io);
        output_file->name = file.name;
        output_file->change_extension(".bmp");
        return output_file;
    }

    file.io.seek(0);
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a CBG image");

    uint16_t width = file.io.read_u16_le();
    uint16_t height = file.io.read_u16_le();
    uint16_t bpp = file.io.read_u16_le();
    file.io.skip(10);

    uint32_t huffman_size = file.io.read_u32_le();
    uint32_t key = file.io.read_u32_le();
    uint32_t freq_table_data_size = file.io.read_u32_le();
    file.io.skip(4);

    uint32_t freq_table[256];
    read_frequency_table(
        file.io,
        freq_table_data_size,
        key,
        freq_table);

    NodeInfo node_info[511];
    int last_node = read_node_info(freq_table, node_info);

    std::unique_ptr<char[]> huffman(new char[huffman_size]);

    BufferedIO buffered_io(file.io);
    BitReader bit_reader(buffered_io);
    decompress_huffman(
        bit_reader,
        last_node,
        node_info,
        huffman_size,
        reinterpret_cast<uint8_t*>(huffman.get()));

    size_t output_size = width * height * (bpp >> 3);
    std::unique_ptr<char[]> output(new char[output_size]);
    decompress_rle(huffman_size, huffman.get(), output.get());
    transform_colors(
        reinterpret_cast<uint8_t*>(output.get()), width, height, bpp);

    std::unique_ptr<Image> image = Image::from_pixels(
        width,
        height,
        std::string(output.get(), output_size),
        bpp_to_image_pixel_format(bpp));
    return image->create_file(file.name);
}
