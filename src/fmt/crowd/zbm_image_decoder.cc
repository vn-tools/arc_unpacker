#include "fmt/crowd/zbm_image_decoder.h"
#include "algo/range.h"
#include "fmt/microsoft/bmp_image_decoder.h"
#include "ptr.h"
#include "util/cyclic_buffer.h"

using namespace au;
using namespace au::fmt::crowd;

static const auto magic = "SZDD"_b;

// different initial pos
static bstr custom_lzss_decompress(
    io::Stream &input_stream, const size_t output_size)
{
    util::CyclicBuffer<0x1000> dict(0xFF0);
    bstr output(output_size);
    auto output_ptr = make_ptr(output);
    u16 control = 0;
    while (output_ptr < output_ptr.end())
    {
        control >>= 1;
        if (!(control & 0x100))
        {
            control = input_stream.read_u8() | 0xFF00;
        }
        if (control & 1)
        {
            *output_ptr++ = input_stream.read_u8();
            dict << output_ptr[-1];
        }
        else
        {
            const auto lo = input_stream.read_u8();
            const auto hi = input_stream.read_u8();
            auto look_behind_pos = lo | ((hi & 0xF0) << 4);
            auto repetitions = (hi & 0xF) + 3;
            while (repetitions-- && output_ptr < output_ptr.end())
            {
                *output_ptr++ = dict[look_behind_pos++];
                dict << output_ptr[-1];
            }
        }
    }
    return output;
}

bool ZbmImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image ZbmImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(10);
    const auto size_orig = input_file.stream.read_u32_le();

    auto data = custom_lzss_decompress(input_file.stream, size_orig);
    for (const auto i : algo::range(std::min<size_t>(100, data.size())))
        data[i] ^= 0xFF;

    const fmt::microsoft::BmpImageDecoder bmp_decoder;
    io::File bmp_file("dummy.bmp", data);
    return bmp_decoder.decode(bmp_file);
}

static auto dummy = fmt::register_fmt<ZbmImageDecoder>("crowd/zbm");
