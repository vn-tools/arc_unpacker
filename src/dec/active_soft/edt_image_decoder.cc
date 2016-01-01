#include "dec/active_soft/edt_image_decoder.h"
#include "algo/range.h"
#include "dec/active_soft/custom_bit_reader.h"
#include "err.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::active_soft;

static const bstr magic = ".TRUE\x8D\x5D\x8C\xCB\x00"_b;
static const bstr diff_magic = ".EDT_DIFF\x00"_b;

static inline u8 clamp(const u8 input, const u8 min, const u8 max)
{
    return std::max<u8>(std::min<u8>(input, max), min);
}

bool EdtImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image EdtImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 4);
    const auto width = input_file.stream.read_u16_le();
    const auto height = input_file.stream.read_u16_le();
    input_file.stream.skip(4);
    const auto meta_size = input_file.stream.read_u32_le();
    const auto data_size = input_file.stream.read_u32_le();
    const auto raw_size = input_file.stream.read_u32_le();

    res::Pixel transparent_color = {0, 0, 0, 0xFF};
    std::string base_file_name;
    if (meta_size)
    {
        io::MemoryStream meta_stream(input_file.stream.read(meta_size));
        if (meta_stream.read(diff_magic.size()) == diff_magic)
        {
            transparent_color
                = res::read_pixel<res::PixelFormat::BGR888>(meta_stream);
            meta_stream.skip(1);
            base_file_name = meta_stream.read_to_eof().str();
        }
    }

    const size_t channels = 3;
    const size_t stride = width * channels;
    const size_t target_size = height * stride;

    std::vector<std::pair<s8, s8>> shift_table;
    for (const auto y : {-4, -3, -2, -1})
    for (const auto x : {-3, -2, -1, 0, 1, 2, 3})
        shift_table.push_back({x, y});
    for (const auto x : {-4, -3, -2, -1})
        shift_table.push_back({x, 0});

    std::vector<size_t> look_behind_table;
    for (const auto shift : shift_table)
        look_behind_table.push_back(
            -(shift.first + shift.second * width) * channels);

    bstr output;
    output.reserve(target_size);

    CustomBitReader bit_reader(input_file.stream.read(data_size));
    io::MemoryStream raw_stream(input_file.stream.read(raw_size));
    output += raw_stream.read(channels);
    while (output.size() < target_size)
    {
        if (!bit_reader.get(1))
        {
            output += raw_stream.read(channels);
            continue;
        }

        if (bit_reader.get(1))
        {
            auto look_behind = channels;
            if (bit_reader.get(1))
            {
                const auto idx = bit_reader.get(2) << 3;
                look_behind = look_behind_table[(0x11'19'17'18 >> idx) & 0xFF];
            }
            if (look_behind > output.size())
                throw err::BadDataOffsetError();
            for (const auto i : algo::range(channels))
            {
                u8 b = clamp(output[output.size() - look_behind], 0x02, 0xFD);
                if (bit_reader.get(1))
                {
                    const int b2 = bit_reader.get(1) + 1;
                    b += bit_reader.get(1) ? b2 : -b2;
                }
                output += static_cast<u8>(b);
            }
        }
        else
        {
            const auto look_behind = look_behind_table[bit_reader.get(5)];
            auto repetitions = bit_reader.get_gamma(0) * channels;
            if (look_behind > output.size())
                throw err::BadDataOffsetError();
            while (repetitions-- && output.size() < target_size)
                output += output[output.size() - look_behind];
        }
    }

    auto image = res::Image(width, height, output, res::PixelFormat::BGR888);
    if (!base_file_name.empty())
        for (auto &c : image)
            if (c == transparent_color)
                c.a = 0;
    return image;
}

static auto _ = dec::register_decoder<EdtImageDecoder>("active-soft/edt");
