#include "fmt/lilim/abm_image_decoder.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::fmt::lilim;

static const bstr magic = "BM"_b;

static bstr decompress_opaque(const bstr &input, const size_t size_hint)
{
    bstr output;
    output.reserve(size_hint);
    io::MemoryStream input_stream(input);

    while (!input_stream.eof())
    {
        const auto control = input_stream.read_u8();

        if (control == 0x00)
        {
            auto repetitions = input_stream.read_u8();
            while (repetitions--)
                output += '\x00';
        }

        else if (control == 0xFF)
        {
            auto repetitions = input_stream.read_u8();
            while (repetitions--)
                output += input_stream.read_u8();
        }

        else
            output += input_stream.read_u8();
    }

    return output;
}

static bstr decompress_alpha(const bstr &input, const size_t size_hint)
{
    bstr output;
    output.reserve(size_hint);
    io::MemoryStream input_stream(input);

    size_t current_channel = 0;
    while (!input_stream.eof())
    {
        const auto control = input_stream.read_u8();

        if (control == 0x00)
        {
            auto repetitions = input_stream.read_u8();
            while (repetitions--)
            {
                output += '\x00';
                current_channel++;
                if (current_channel == 3)
                {
                    output += '\x00';
                    current_channel = 0;
                }
            }
        }

        else if (control == 0xFF)
        {
            auto repetitions = input_stream.read_u8();
            while (repetitions--)
            {
                output += input_stream.read_u8();
                current_channel++;
                if (current_channel == 3)
                {
                    output += '\xFF';
                    current_channel = 0;
                }
            }
        }

        else
        {
            output += input_stream.read_u8();
            current_channel++;
            if (current_channel == 3)
            {
                output += control;
                current_channel = 0;
            }
        }
    }

    return output;
}

bool AbmImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("abm")
        && input_file.stream.read(magic.size()) == magic;
}

res::Image AbmImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(18);
    const auto width = input_file.stream.read_u32_le();
    const auto height = input_file.stream.read_u32_le();
    input_file.stream.skip(2);
    const s16 depth = input_file.stream.read_u16_le();

    const auto size = width * height * 4;
    input_file.stream.seek(54);
    if (depth == -8)
    {
        res::Palette palette(
            256, input_file.stream.read(256 * 4), res::PixelFormat::BGR888X);
        return res::Image(
            width,
            height,
            decompress_opaque(input_file.stream.read_to_eof(), size),
            palette);
    }
    else if (depth == 32)
    {
        return res::Image(
            width,
            height,
            decompress_alpha(input_file.stream.read_to_eof(), size),
            res::PixelFormat::BGRA8888);
    }
    else
        throw err::UnsupportedBitDepthError(depth);
}

static auto dummy = fmt::register_fmt<AbmImageDecoder>("lilim/abm");
