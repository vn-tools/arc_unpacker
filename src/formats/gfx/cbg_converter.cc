// CBG image
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: -
// Archives:  ARC

#include <cassert>
#include <cstring>
#include "formats/gfx/cbg_converter.h"
#include "formats/image.h"
#include "io.h"
#include "logger.h"

static const char *cbg_magic = "CompressedBG___\x00";
static const size_t cbg_magic_length = 16;

typedef struct
{
    bool valid;
    uint32_t frequency;
    int left_node;
    int right_node;
} CbgNodeInfo;

static uint32_t cbg_get_key(uint32_t *pkey)
{
    uint32_t key = *pkey;
    uint32_t tmp1 = 0x4e35 * (key & 0xffff);
    uint32_t tmp2 = 0x4e35 * (key >> 16);
    uint32_t tmp = 0x15a * key + tmp2 + (tmp1 >> 16);
    *pkey = (tmp << 16) + (tmp1 & 0xffff) + 1;
    return tmp & 0x7fff;
}

static void cbg_decrypt(char *input, uint32_t decrypt_size, uint32_t key)
{
    uint32_t i;
    for (i = 0; i < decrypt_size; i ++)
    {
        *input ++ -= (char) cbg_get_key(&key);
    }
}

static uint32_t cbg_read_variable_data(char **input, char *input_guardian)
{
    char current;
    uint32_t result = 0;
    uint32_t shift = 0;
    do
    {
        current = **input;
        (*input) ++;
        assert(*input <= input_guardian);
        result |= (current & 0x7f) << shift;
        shift += 7;
    } while (current & 0x80);
    return result;
}

static void cbg_read_frequency_table(
    IO *io,
    uint32_t raw_data_size,
    uint32_t key,
    uint32_t frequency_table[])
{
    char *raw_data = new char[raw_data_size];
    assert(raw_data != nullptr);
    io_read_string(io, raw_data, raw_data_size);

    cbg_decrypt(raw_data, raw_data_size, key);

    int i;
    char *raw_data_ptr = raw_data;
    for (i = 0; i < 256; i ++)
    {
        frequency_table[i] = cbg_read_variable_data(
            &raw_data_ptr,
            raw_data + raw_data_size);
    }

    delete []raw_data;
}

static int cbg_read_node_info(
    uint32_t frequency_table[],
    CbgNodeInfo node_info[])
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

static void cbg_decompress_huffman(
    IO *io,
    int last_node,
    CbgNodeInfo node_info[],
    uint32_t huffman_size,
    uint8_t *huffman)
{
    assert(io != nullptr);
    assert(node_info != nullptr);
    uint32_t root = last_node;
    uint8_t mask = 0x80;
    size_t i;

    for (i = 0; i < huffman_size; i ++)
    {
        int node = root;
        while (node >= 256)
        {
            node = io_read_u8(io) & mask
                ? node_info[node].right_node
                : node_info[node].left_node;
            mask >>= 1;
            if (!mask)
                mask = 0x80;
            else
                io_skip(io, -1);
        }
        huffman[i] = node;
    }
}

static void cbg_decompress_rle(
    uint32_t huffman_size,
    char *huffman,
    char *output)
{
    assert(huffman != nullptr);
    assert(output != nullptr);
    char *huffman_ptr = huffman;
    char *huffman_guardian = huffman + huffman_size;
    bool zero_flag = false;
    while (huffman_ptr < huffman_guardian)
    {
        uint32_t length = cbg_read_variable_data(
            &huffman_ptr,
            huffman_guardian);
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

static void cbg_transform_colors(
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

static PixelFormat bpp_to_image_pixel_format(short bpp)
{
    switch (bpp)
    {
        case 8:
            return IMAGE_PIXEL_FORMAT_GRAYSCALE;
        case 24:
            return IMAGE_PIXEL_FORMAT_BGR;
        case 32:
            return IMAGE_PIXEL_FORMAT_BGRA;
    }
    log_error("CBG: Unsupported BPP: %d", bpp);
    return (PixelFormat)0;
}

static bool cbg_decode(Converter *converter, VirtualFile *file)
{
    assert(converter != nullptr);
    assert(file != nullptr);

    char magic[cbg_magic_length];
    io_read_string(file->io, magic, cbg_magic_length);
    if (memcmp(magic, cbg_magic, cbg_magic_length) != 0)
    {
        log_error("CBG: Not a CBG image");
        return false;
    }

    uint16_t width = io_read_u16_le(file->io);
    uint16_t height = io_read_u16_le(file->io);
    uint16_t bpp = io_read_u16_le(file->io);
    io_skip(file->io, 10);

    if (!bpp_to_image_pixel_format(bpp))
        return false;

    uint32_t huffman_size = io_read_u32_le(file->io);
    uint32_t key = io_read_u32_le(file->io);
    uint32_t freq_table_data_size = io_read_u32_le(file->io);
    io_skip(file->io, 4);

    uint32_t freq_table[256];
    cbg_read_frequency_table(
        file->io,
        freq_table_data_size,
        key,
        freq_table);

    CbgNodeInfo node_info[511];
    int last_node = cbg_read_node_info(freq_table, node_info);

    bool result;
    char *huffman = new char[huffman_size];
    if (huffman == nullptr)
    {
        log_error("CBG: Failed to allocate %d bytes", huffman_size);
        result = false;
    }
    else
    {
        cbg_decompress_huffman(
            file->io,
            last_node,
            node_info,
            huffman_size,
            (uint8_t*)huffman);

        size_t output_size = width * height * (bpp >> 3);
        char *output = new char[output_size];
        if (output == nullptr)
        {
            log_error("CBG: Failed to allocate %d bytes", output_size);
            result = false;
        }
        else
        {
            cbg_decompress_rle(huffman_size, huffman, output);
            delete []huffman;
            cbg_transform_colors((uint8_t*)output, width, height, bpp);

            Image *image = image_create_from_pixels(
                width,
                height,
                output,
                output_size,
                bpp_to_image_pixel_format(bpp));
            image_update_file(image, file);
            image_destroy(image);

            delete []output;
        }
    }

    return result;
}

Converter *cbg_converter_create()
{
    Converter *converter = converter_create();
    converter->decode = &cbg_decode;
    return converter;
}
