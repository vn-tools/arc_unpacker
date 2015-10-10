#include "fmt/bgi/cbg/cbg1_decoder.h"
#include "err.h"
#include "fmt/bgi/cbg/cbg_common.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::bgi::cbg;

static bstr decompress_huffman(
    io::BitReader &bit_reader, const Tree &tree, size_t output_size)
{
    bstr output(output_size);
    for (auto i : util::range(output.size()))
        output[i] = tree.get_leaf(bit_reader);
    return output;
}

static bstr decompress_rle(bstr &input, size_t output_size)
{
    io::BufferedIO input_io(input);

    bstr output(output_size);
    auto output_ptr = output.get<u8>();
    auto output_end = output.get<const u8>() + output.size();

    bool zero_flag = false;
    while (!input_io.eof())
    {
        u32 size = read_variable_data(input_io);
        if (output_ptr + size >= output_end)
            size = output_end - output_ptr;

        if (zero_flag)
        {
            for (auto i : util::range(size))
                *output_ptr++ = 0;
        }
        else
        {
            for (auto i : util::range(size))
                *output_ptr++ = input_io.read_u8();
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

    // add left and top to all other pixels
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
                *input_ptr += (*left + *above) >> 1;
                input_ptr++;
                above++;
                left++;
            }
        }
    }
}

static pix::Format bpp_to_pixel_format(int bpp)
{
    switch (bpp)
    {
        case 8:
            return pix::Format::Gray8;
        case 24:
            return pix::Format::BGR888;
        case 32:
            return pix::Format::BGRA8888;
    }
    throw err::UnsupportedBitDepthError(bpp);
}

std::unique_ptr<pix::Grid> Cbg1Decoder::decode(io::IO &io) const
{
    auto width = io.read_u16_le();
    auto height = io.read_u16_le();
    auto bpp = io.read_u32_le();
    io.skip(8);

    auto huffman_size = io.read_u32_le();
    io::BufferedIO decrypted_io(read_decrypted_data(io));
    auto raw_data = io.read_to_eof();

    auto freq_table = read_freq_table(decrypted_io, 256);
    auto tree = build_tree(freq_table, false);

    io::BitReader bit_reader(raw_data);
    auto output = decompress_huffman(bit_reader, tree, huffman_size);
    auto pixel_data = decompress_rle(output, width * height * (bpp >> 3));
    transform_colors(pixel_data, width, height, bpp);

    auto format = bpp_to_pixel_format(bpp);
    return std::make_unique<pix::Grid>(width, height, pixel_data, format);
}
