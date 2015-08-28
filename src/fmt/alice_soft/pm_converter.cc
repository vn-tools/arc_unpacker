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
#include "io/buffered_io.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const bstr magic = "PM\x02\x00"_b;

static bstr decompress(const bstr &input, size_t width, size_t height)
{
    bstr output;
    output.resize(width * height);
    u8 *output_ptr = output.get<u8>();
    const u8 *output_end = output_ptr + output.size();
    const u8 *input_ptr = input.get<const u8>();
    const u8 *input_end = input_ptr + input.size();

    while (input_ptr < input_end && output_ptr < output_end)
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
                while (n-- && output_ptr < output_end)
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
                while (n-- && output_ptr < output_end)
                    *output_ptr++ = *input_ptr;
                input_ptr++;
                break;
            }

            case 0xFE:
            {
                size_t n = *input_ptr++ + 3;
                while (n-- && output_ptr < output_end)
                {
                    *output_ptr = *(output_ptr - width * 2);
                    output_ptr++;
                }
                break;
            }

            case 0xFF:
            {
                size_t n = *input_ptr++ + 3;
                while (n-- && output_ptr < output_end)
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

    return output;
}

static std::unique_ptr<util::Image> decode_to_image(io::IO &io)
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
    auto palette_data = io.read(256 * 3);

    io.seek(data_offset);
    auto pixel_data = decompress(io.read_to_eof(), width, height);

    pix::Palette palette(256, palette_data, pix::Format::BGR888);
    pix::Grid pixels(width, height, pixel_data, palette);
    return util::Image::from_pixels(pixels);
}

std::unique_ptr<util::Image> PmConverter::decode_to_image(
    const bstr &data) const
{
    io::BufferedIO tmp_io(data);
    return ::decode_to_image(tmp_io);
}

bool PmConverter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> PmConverter::decode_internal(File &file) const
{
    auto image = ::decode_to_image(file.io);
    return image->create_file(file.name);
}

static auto dummy = fmt::Registry::add<PmConverter>("alice/pm");
