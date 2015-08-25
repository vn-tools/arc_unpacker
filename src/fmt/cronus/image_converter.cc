// Cronus image
//
// Company:   Cronus
// Engine:    -
// Extension: -
// Archives:  PAK
//
// Known games:
// - Doki Doki Princess

#include "fmt/cronus/image_converter.h"
#include "util/image.h"
#include "util/require.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::cronus;

static const int key1 = 0xA53CC35A;
static const int key2 = 0x35421005;
static const int key3 = 0xCF42355D;

static void decrypt(bstr &input, size_t encrypted_size)
{
    u16 *input_ptr = input.get<u16>();
    const u16 *input_end = input.end<const u16>();
    size_t repetitions = encrypted_size >> 1;
    while (repetitions-- && input_ptr < input_end)
    {
        *input_ptr = ((*input_ptr << 8) | (*input_ptr >> 8)) ^ 0x33CC;
        input_ptr++;
    }
}

//TODO: this is identical to Libido's ARC archive; move it to pack/lzss
static bstr decompress(const bstr &input, size_t output_size)
{
    bstr output;
    output.resize(output_size);

    const size_t dict_size = 0x1000;
    size_t dict_pos = 0xFEE;
    u8 dict[dict_size];
    for (auto i : util::range(dict_size))
        dict[i] = 0;

    u8 *output_ptr = output.get<u8>();
    const u8 *output_end = output.end<const u8>();
    const u8 *input_ptr = input.get<const u8>();
    const u8 *input_end = input.end<const u8>();

    u16 control = 0;
    int input_pos = 0;
    while (output_ptr < output_end && input_ptr < input_end)
    {
        control >>= 1;
        if (!(control & 0x100))
        {
            control = *input_ptr++ | 0xFF00;
            if (input_ptr >= input_end)
                break;
        }
        if (control & 1)
        {
            dict[dict_pos++] = *output_ptr++ = *input_ptr++;
            dict_pos %= dict_size;
            if (input_ptr >= input_end)
                break;
        }
        else
        {
            u8 tmp1 = *input_ptr++;
            if (input_ptr >= input_end)
                break;
            u8 tmp2 = *input_ptr++;
            if (input_ptr >= input_end)
                break;

            u16 look_behind_pos = (((tmp2 & 0xF0) << 4) | tmp1) % dict_size;
            u16 repetitions = (tmp2 & 0xF) + 3;
            while (repetitions-- && output_ptr < output_end)
            {
                dict[dict_pos++] = *output_ptr++ = dict[look_behind_pos++];
                look_behind_pos %= dict_size;
                dict_pos %= dict_size;
            }
        }
    }
    return output;
}

bool ImageConverter::is_recognized_internal(File &file) const
{
    auto width = file.io.read_u32_le() ^ key1;
    auto height = file.io.read_u32_le() ^ key2;
    auto bpp = file.io.read_u32_le();
    file.io.skip(4);
    auto output_size = file.io.read_u32_le() ^ key3;

    size_t expected_output_size = width * height;
    if (bpp == 8)
        expected_output_size += 1024;
    else if (bpp == 24)
        expected_output_size *= 3;
    else
        return false;

    return output_size == expected_output_size;
}

std::unique_ptr<File> ImageConverter::decode_internal(File &file) const
{
    auto width = file.io.read_u32_le() ^ key1;
    auto height = file.io.read_u32_le() ^ key2;
    auto bpp = file.io.read_u32_le();
    auto unk0 = file.io.read_u32_le();
    auto output_size = file.io.read_u32_le() ^ key3;
    auto unk1 = file.io.read_u32_le();

    auto data = file.io.read_to_eof();
    decrypt(data, output_size);
    data = decompress(data, output_size);

    if (bpp == 8)
    {
        pix::Palette palette(256, data, pix::Format::BGRA8888);
        for (auto i : util::range(palette.size()))
            palette[i].a = 0xFF;
        pix::Grid pixels(width, height, data.substr(1024), palette);
        pixels.flip();
        return util::Image::from_pixels(pixels)->create_file(file.name);
    }
    else if (bpp == 24)
    {
        pix::Grid pixels(width, height, data, pix::Format::BGR888);
        pixels.flip();
        return util::Image::from_pixels(pixels)->create_file(file.name);
    }
    else
        util::fail("Unsupported BPP");
}
