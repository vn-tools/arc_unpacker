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
#include "util/pack/lzss.h"
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
    data = util::pack::lzss_decompress_bytewise(data, output_size);

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
