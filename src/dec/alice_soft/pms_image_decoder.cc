#include "dec/alice_soft/pms_image_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::alice_soft;

static const bstr magic1 = "PM\x01\x00"_b;
static const bstr magic2 = "PM\x02\x00"_b;

bstr PmsImageDecoder::decompress_8bit(
    io::IStream &input_stream, const size_t width, const size_t height)
{
    bstr output(width * height);
    auto output_ptr = output.get<u8>();
    auto output_end = output.end<u8>();

    while (output_ptr < output_end)
    {
        u8 c = input_stream.read_u8();

        switch (c)
        {
            case 0xFF:
            {
                auto n = input_stream.read_u8() + 3;
                while (n-- && output_ptr < output_end)
                {
                    *output_ptr = output_ptr[-width];
                    output_ptr++;
                }
                break;
            }

            case 0xFE:
            {
                auto n = input_stream.read_u8() + 3;
                while (n-- && output_ptr < output_end)
                {
                    *output_ptr = output_ptr[-width * 2];
                    output_ptr++;
                }
                break;
            }

            case 0xFD:
            {
                auto n = input_stream.read_u8() + 4;
                auto color = input_stream.read_u8();
                while (n-- && output_ptr < output_end)
                    *output_ptr++ = color;
                break;
            }

            case 0xFC:
            {
                auto n = input_stream.read_u8() + 3;
                auto color1 = input_stream.read_u8();
                auto color2 = input_stream.read_u8();
                while (n-- && output_ptr < output_end)
                {
                    *output_ptr++ = color1;
                    *output_ptr++ = color2;
                }
                break;
            }

            case 0xF8:
                *output_ptr++ = input_stream.read_u8();
                break;

            default:
                *output_ptr++ = c;
        }
    }

    return output;
}

bstr PmsImageDecoder::decompress_16bit(
    io::IStream &input_stream, size_t width, size_t height)
{
    bstr output(width * height * 2);
    auto output_ptr = output.get<u16>();
    auto output_end = output.end<const u16>();

    while (output_ptr < output_end)
    {
        u8 c = input_stream.read_u8();

        switch (c)
        {
            case 0xFF:
            {
                auto n = input_stream.read_u8() + 2;
                while (n-- && output_ptr < output_end)
                {
                    *output_ptr = output_ptr[-width];
                    output_ptr++;
                }
                break;
            }

            case 0xFE:
            {
                auto n = input_stream.read_u8() + 2;
                while (n-- && output_ptr < output_end)
                {
                    *output_ptr = output_ptr[-width * 2];
                    output_ptr++;
                }
                break;
            }

            case 0xFD:
            {
                auto size = input_stream.read_u8() + 3;
                auto color = input_stream.read_u16_le();
                while (size-- && output_ptr < output_end)
                    *output_ptr++ = color;
                break;
            }

            case 0xFC:
            {
                auto n = input_stream.read_u8() + 2;
                auto color1 = input_stream.read_u16_le();
                auto color2 = input_stream.read_u16_le();
                while (n-- && output_ptr < output_end)
                {
                    *output_ptr++ = color1;
                    *output_ptr++ = color2;
                }
                break;
            }

            case 0xFB:
                *output_ptr = output_ptr[-width - 1];
                output_ptr++;
                break;

            case 0xFA:
                *output_ptr = output_ptr[-width + 1];
                output_ptr++;
                break;

            case 0xF9:
            {
                auto n = input_stream.read_u8() + 1;
                auto byte1 = input_stream.read_u8();
                auto half1
                    = ((byte1 & 0b11100000) << 8)
                    | ((byte1 & 0b00011000) << 6)
                    | ((byte1 & 0b00000111) << 2);
                while (n-- && output_ptr < output_end)
                {
                    auto byte2 = input_stream.read_u8();
                    auto half2
                        = ((byte2 & 0b11000000) << 5)
                        | ((byte2 & 0b00111100) << 3)
                        | ((byte2 & 0b00000011) << 0);
                    *output_ptr++ = half1 | half2;
                }
                break;
            }

            case 0xF8:
                *output_ptr++ = input_stream.read_u16_le();
                break;

            default:
                *output_ptr++ = c | (input_stream.read_u8() << 8);
                break;
        }
    }

    return output;
}

bool PmsImageDecoder::is_recognized_impl(io::File &input_file) const
{
    if (input_file.stream.read(magic1.size()) == magic1)
        return true;
    input_file.stream.seek(0);
    return input_file.stream.read(magic2.size()) == magic2;
}

res::Image PmsImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(2);
    auto version = input_file.stream.read_u16_le();
    input_file.stream.skip(2);
    auto depth = input_file.stream.read_u16_le();
    input_file.stream.skip(4 * 4);
    auto width = input_file.stream.read_u32_le();
    auto height = input_file.stream.read_u32_le();
    auto data_offset = input_file.stream.read_u32_le();
    auto extra_data_offset = input_file.stream.read_u32_le();

    std::unique_ptr<res::Image> image;

    if (depth == 8)
    {
        input_file.stream.seek(data_offset);
        auto pixel_data = decompress_8bit(input_file.stream, width, height);
        input_file.stream.seek(extra_data_offset);
        res::Palette palette(
            256,
            input_file.stream,
            version == 1 ? res::PixelFormat::RGB888 : res::PixelFormat::BGR888);
        image.reset(new res::Image(width, height, pixel_data, palette));
    }
    else if (depth == 16)
    {
        input_file.stream.seek(data_offset);
        auto pixel_data = decompress_16bit(input_file.stream, width, height);
        image.reset(new res::Image(
            width, height, pixel_data, res::PixelFormat::BGR565));
        if (extra_data_offset)
        {
            input_file.stream.seek(extra_data_offset);
            auto mask_data = decompress_8bit(input_file.stream, width, height);
            res::Image mask(width, height, mask_data, res::PixelFormat::Gray8);
            image->apply_mask(mask);
        }
    }
    else
    {
        throw err::UnsupportedBitDepthError(depth);
    }

    return *image;
}

static auto _ = dec::register_decoder<PmsImageDecoder>("alice-soft/pms");
