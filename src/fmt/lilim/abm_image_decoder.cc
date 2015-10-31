#include "fmt/lilim/abm_image_decoder.h"
#include "err.h"
#include "io/buffered_io.h"

using namespace au;
using namespace au::fmt::lilim;

static const bstr magic = "BM"_b;

static bstr decompress_opaque(const bstr &input, const size_t size_hint)
{
    bstr output;
    output.reserve(size_hint);
    io::BufferedIO input_io(input);

    while (!input_io.eof())
    {
        const auto control = input_io.read_u8();

        if (control == 0x00)
        {
            auto repetitions = input_io.read_u8();
            while (repetitions--)
                output += '\x00';
        }

        else if (control == 0xFF)
        {
            auto repetitions = input_io.read_u8();
            while (repetitions--)
                output += input_io.read_u8();
        }

        else
            output += input_io.read_u8();
    }

    return output;
}

static bstr decompress_alpha(const bstr &input, const size_t size_hint)
{
    bstr output;
    output.reserve(size_hint);
    io::BufferedIO input_io(input);

    size_t current_channel = 0;
    while (!input_io.eof())
    {
        const auto control = input_io.read_u8();

        if (control == 0x00)
        {
            auto repetitions = input_io.read_u8();
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
            auto repetitions = input_io.read_u8();
            while (repetitions--)
            {
                output += input_io.read_u8();
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
            output += input_io.read_u8();
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

bool AbmImageDecoder::is_recognized_impl(File &file) const
{
    return file.has_extension("abm") && file.io.read(magic.size()) == magic;
}

pix::Grid AbmImageDecoder::decode_impl(File &file) const
{
    file.io.seek(18);
    const auto width = file.io.read_u32_le();
    const auto height = file.io.read_u32_le();
    file.io.skip(2);
    const s16 depth = file.io.read_u16_le();

    if (depth == -8)
    {
        const auto size = width * height * 4;
        file.io.seek(54);
        pix::Palette palette(256, file.io, pix::Format::BGR888X);
        const auto pixel_data = decompress_opaque(file.io.read_to_eof(), size);
        return pix::Grid(width, height, pixel_data, palette);
    }
    else if (depth == 32)
    {
        const auto size = width * height * 4;
        file.io.seek(54);
        const auto pixel_data = decompress_alpha(file.io.read_to_eof(), size);
        return pix::Grid(width, height, pixel_data, pix::Format::BGRA8888);
    }
    else
        throw err::UnsupportedBitDepthError(depth);
}

static auto dummy = fmt::register_fmt<AbmImageDecoder>("lilim/abm");
