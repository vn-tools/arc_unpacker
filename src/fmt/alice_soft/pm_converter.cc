// PM image mask
//
// Company:   Alice Soft
// Engine:    -
// Extension: -
// Archives:  AJP
//
// Known games:
// - Daiakuji

#include <array>
#include "fmt/alice_soft/pm_converter.h"
#include "util/colors.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt;
using namespace au::fmt::alice_soft;

static const std::string magic = "PM\x02\x00"_s;

static std::string decompress(
    const std::string &input, size_t width, size_t height)
{
    auto output_size = width * height;
    std::unique_ptr<u8[]> output(new u8[output_size]);
    u8 *output_ptr = output.get();
    const u8 *output_guardian = output_ptr + output_size;
    const u8 *input_ptr = reinterpret_cast<const u8*>(input.data());
    const u8 *input_guardian = input_ptr + input.size();

    while (input_ptr < input_guardian && output_ptr < output_guardian)
    {
        u8 c = *input_ptr++;

        switch (c)
        {
            case 0xF8:
                *output_ptr++ = *input_ptr++;
                break;

            case 0xFC:
            {
                size_t n = *input_ptr++ + 3;
                while (n-- && output_ptr < output_guardian)
                {
                    *output_ptr++ = *input_ptr;
                    *output_ptr++ = *(input_ptr + 1);
                }
                input_ptr += 2;
                break;
            }

            case 0xFD:
            {
                size_t n = *input_ptr++ + 4;
                while (n-- && output_ptr < output_guardian)
                    *output_ptr++ = *input_ptr;
                input_ptr++;
                break;
            }

            case 0xFE:
            {
                size_t n = *input_ptr++ + 3;
                while (n-- && output_ptr < output_guardian)
                {
                    *output_ptr = *(output_ptr - width * 2);
                    output_ptr++;
                }
                break;
            }

            case 0xFF:
            {
                size_t n = *input_ptr++ + 3;
                while (n-- && output_ptr < output_guardian)
                {
                    *output_ptr = *(output_ptr - width);
                    output_ptr++;
                }
                break;
            }

            default:
                *output_ptr++ = c;
        }
    }

    return std::string(reinterpret_cast<char*>(output.get()), output_size);
}

std::unique_ptr<util::Image> PmConverter::decode_to_image(io::IO &io) const
{
    io.skip(magic.size());
    io.skip(2);
    auto depth = io.read_u16_le();
    if (depth != 8)
    {
        throw std::runtime_error(util::format(
            "Unsupported bit depth: %d", depth));
    }
    io.skip(4 * 4);
    auto width = io.read_u32_le();
    auto height = io.read_u32_le();
    auto data_offset = io.read_u32_le();
    auto palette_offset = io.read_u32_le();

    io.seek(palette_offset);
    std::array<u32, 256> palette;
    for (auto i : util::range(256))
    {
        auto r = io.read_u8();
        auto g = io.read_u8();
        auto b = io.read_u8();
        palette[i] = util::color::rgb888(r, g, b);
    }

    io.seek(data_offset);
    auto data = decompress(io.read_until_end(), width, height);

    std::unique_ptr<u32[]> pixels(new u32[width * height]);
    for (auto i : util::range(width * height))
        pixels[i] = palette[static_cast<u8>(data[i])];

    return util::Image::from_pixels(
        width, height,
        std::string(reinterpret_cast<char*>(pixels.get()), width * height * 4),
        util::PixelFormat::BGRA);
}

bool PmConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> PmConverter::decode_internal(File &file) const
{
    auto image = decode_to_image(file.io);
    return image->create_file(file.name);
}
