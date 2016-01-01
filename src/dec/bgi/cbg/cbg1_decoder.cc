#include "dec/bgi/cbg/cbg1_decoder.h"
#include "algo/range.h"
#include "dec/bgi/cbg/cbg_common.h"
#include "err.h"
#include "io/memory_stream.h"
#include "io/msb_bit_reader.h"

using namespace au;
using namespace au::dec::bgi::cbg;

static bstr decompress_huffman(
    io::IBitReader &bit_reader, const Tree &tree, size_t output_size)
{
    bstr output(output_size);
    for (auto i : algo::range(output.size()))
        output[i] = tree.get_leaf(bit_reader);
    return output;
}

static bstr decompress_rle(bstr &input, size_t output_size)
{
    io::MemoryStream input_stream(input);

    bstr output(output_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.get<const u8>() + output.size();

    bool zero_flag = false;
    while (!input_stream.eof())
    {
        u32 size = read_variable_data(input_stream);
        if (output_ptr + size >= output_end)
            size = output_end - output_ptr;

        if (zero_flag)
        {
            for (auto i : algo::range(size))
                *output_ptr++ = 0;
        }
        else
        {
            for (auto i : algo::range(size))
                *output_ptr++ = input_stream.read_u8();
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

    // ignore 0,0
    input_ptr += channels;
    above += channels;
    left += channels;

    // add left to first row
    for (auto x : algo::range(1, width))
    {
        for (auto i : algo::range(channels))
        {
            *input_ptr += input_ptr[-channels];
            input_ptr++;
            above++;
            left++;
        }
    }

    // add left and top to all other pixels
    for (auto y : algo::range(1, height))
    {
        for (auto i : algo::range(channels))
        {
            *input_ptr += *above;
            input_ptr++;
            above++;
            left++;
        }

        for (auto x : algo::range(1, width))
        {
            for (auto i : algo::range(channels))
            {
                *input_ptr += (*left + *above) >> 1;
                input_ptr++;
                above++;
                left++;
            }
        }
    }
}

static res::PixelFormat bpp_to_pixel_format(int bpp)
{
    switch (bpp)
    {
        case 8:
            return res::PixelFormat::Gray8;
        case 24:
            return res::PixelFormat::BGR888;
        case 32:
            return res::PixelFormat::BGRA8888;
    }
    throw err::UnsupportedBitDepthError(bpp);
}

std::unique_ptr<res::Image> Cbg1Decoder::decode(io::IStream &input_stream) const
{
    auto width = input_stream.read_u16_le();
    auto height = input_stream.read_u16_le();
    auto bpp = input_stream.read_u32_le();
    input_stream.skip(8);

    auto huffman_size = input_stream.read_u32_le();
    io::MemoryStream decrypted_stream(read_decrypted_data(input_stream));
    auto raw_data = input_stream.read_to_eof();

    auto freq_table = read_freq_table(decrypted_stream, 256);
    auto tree = build_tree(freq_table, false);

    io::MsbBitReader bit_reader(raw_data);
    auto output = decompress_huffman(bit_reader, tree, huffman_size);
    auto pixel_data = decompress_rle(output, width * height * (bpp >> 3));
    transform_colors(pixel_data, width, height, bpp);

    auto format = bpp_to_pixel_format(bpp);
    return std::make_unique<res::Image>(width, height, pixel_data, format);
}
