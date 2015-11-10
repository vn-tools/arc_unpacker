#include "fmt/leaf/lc3_image_decoder.h"
#include "util/pack/lzss.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LEAFC64\x00"_b;

// Modified LZSS routine (the bit shifts proceed in opposite direction)
static bstr custom_lzss_decompress(const bstr &input, size_t output_size)
{
    bstr output(output_size);

    const size_t dict_size = 0x1000;
    size_t dict_pos = 0xFEE;
    u8 dict[dict_size] = { 0 };

    u8 *output_ptr = output.get<u8>();
    const u8 *output_end = output.end<const u8>();
    const u8 *input_ptr = input.get<const u8>();
    const u8 *input_end = input.end<const u8>();

    u16 control = 0;
    while (output_ptr < output_end && input_ptr < input_end)
    {
        control <<= 1;
        if (!(control & 0x80))
            control = (*input_ptr++ << 8) | 0xFF;

        if ((control >> 15) & 1)
        {
            auto v = *input_ptr++;
            dict[dict_pos++] = *output_ptr++ = v;
            dict_pos %= dict_size;
        }
        else
        {
            if (input_ptr + 2 > input_end)
                break;
            u16 tmp = reinterpret_cast<const u16&>(*input_ptr);
            input_ptr += 2;

            u16 look_behind_pos = (tmp >> 4) % dict_size;
            u16 repetitions = (tmp & 0xF) + 3;
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

static bstr get_data(io::IO &io, const size_t size_comp, const size_t size_orig)
{
    auto data = io.read(size_comp);
    for (auto &c : data)
        c ^= 0xFF;
    return custom_lzss_decompress(data, size_orig);
}

bool Lc3ImageDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

pix::Grid Lc3ImageDecoder::decode_impl(File &file) const
{
    file.io.seek(magic.size());
    file.io.skip(4);
    const auto width = file.io.read_u16_le();
    const auto height = file.io.read_u16_le();

    const auto alpha_pos = file.io.read_u32_le();
    const auto color_pos = file.io.read_u32_le();

    file.io.seek(color_pos);
    const auto color_data
        = get_data(file.io, file.io.size() - color_pos, width * height * 2);
    pix::Grid image(width, height, color_data, pix::Format::BGR555X);

    file.io.seek(alpha_pos);
    auto mask_data
        = get_data(file.io, color_pos - alpha_pos, width * height);
    for (auto &c : mask_data)
        c <<= 3;
    pix::Grid mask(width, height, mask_data, pix::Format::Gray8);

    image.apply_mask(mask);
    image.flip_vertically();
    return image;
}

static auto dummy = fmt::register_fmt<Lc3ImageDecoder>("leaf/lc3");
