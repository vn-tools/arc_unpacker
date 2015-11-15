#include "fmt/active_soft/edt_image_decoder.h"
#include "err.h"
#include "fmt/active_soft/custom_bit_reader.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::active_soft;

static const bstr magic = ".TRUE\x8D\x5D\x8C\xCB\x00"_b;
static const bstr diff_magic = ".EDT_DIFF\x00"_b;

static inline u8 clamp(const u8 input, const u8 min, const u8 max)
{
    return std::max<u8>(std::min<u8>(input, max), min);
}

bool EdtImageDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

pix::Grid EdtImageDecoder::decode_impl(File &file) const
{
    file.io.seek(magic.size() + 4);
    const auto width = file.io.read_u16_le();
    const auto height = file.io.read_u16_le();
    file.io.skip(4);
    const auto meta_size = file.io.read_u32_le();
    const auto data_size = file.io.read_u32_le();
    const auto raw_size = file.io.read_u32_le();

    pix::Pixel transparent_color = {0, 0, 0, 0xFF};
    std::string base_file_name;
    if (meta_size)
    {
        io::BufferedIO meta_io(file.io.read(meta_size));
        if (meta_io.read(diff_magic.size()) == diff_magic)
        {
            transparent_color = pix::read<pix::Format::BGR888>(meta_io);
            meta_io.skip(1);
            base_file_name = meta_io.read_to_eof().str();
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

    CustomBitReader bit_reader(file.io.read(data_size));
    io::BufferedIO raw_io(file.io.read(raw_size));
    output += raw_io.read(channels);
    while (output.size() < target_size)
    {
        if (!bit_reader.get(1))
        {
            output += raw_io.read(channels);
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
            for (const auto i : util::range(channels))
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
            auto repetitions = bit_reader.get_variable_integer() * channels;
            if (look_behind > output.size())
                throw err::BadDataOffsetError();
            while (repetitions-- && output.size() < target_size)
                output += output[output.size() - look_behind];
        }
    }

    auto ret = pix::Grid(width, height, output, pix::Format::BGR888);
    if (!base_file_name.empty())
        for (auto &c : ret)
            if (c == transparent_color)
                c.a = 0;
    return ret;
}

static auto dummy = fmt::register_fmt<EdtImageDecoder>("active-soft/edt");
