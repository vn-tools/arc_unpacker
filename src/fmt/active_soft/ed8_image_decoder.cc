#include "fmt/active_soft/ed8_image_decoder.h"
#include "err.h"
#include "fmt/active_soft/custom_bit_reader.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::active_soft;

static const bstr magic = ".8Bit\x8D\x5D\x8C\xCB\x00"_b;

bool Ed8ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

pix::Grid Ed8ImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size() + 4);
    const auto width = input_file.stream.read_u16_le();
    const auto height = input_file.stream.read_u16_le();
    const auto palette_size = input_file.stream.read_u32_le();
    const auto data_size = input_file.stream.read_u32_le();
    const pix::Palette palette(
        palette_size, input_file.stream, pix::Format::BGR888);

    static const std::vector<std::pair<s8, s8>> shift_table
        {
            {-1,  0}, {+0, -1}, {-2,  0}, {-1, -1},
            {+1, -1}, {+0, -2}, {-2, -1}, {+2, -1},
            {-2, -2}, {-1, -2}, {+1, -2}, {+2, -2},
            {+0, -3}, {-1, -3},
        };

    std::vector<size_t> look_behind_table;
    for (const auto shift : shift_table)
        look_behind_table.push_back(shift.first + shift.second * width);

    bstr output(width * height);
    auto output_ptr = output.get<u8>();
    const auto output_start = output.get<const u8>();
    const auto output_end = output.end<const u8>();

    CustomBitReader bit_reader(input_file.stream.read(data_size));
    while (output_ptr < output_end)
    {
        *output_ptr++ = bit_reader.get(8);
        if (output_ptr >= output_end)
            break;
        if (bit_reader.get(1))
            continue;
        int last_idx = -1;
        while (output_ptr < output_end)
        {
            int idx = 0;
            if (bit_reader.get(1))
            {
                if (bit_reader.get(1))
                    idx = bit_reader.get(1) + 1;
                idx = (idx << 1) + bit_reader.get(1) + 1;
            }
            idx = (idx << 1) + bit_reader.get(1);

            if (idx == last_idx)
                break;
            last_idx = idx;

            int repetitions = bit_reader.get_variable_integer();
            if (idx >= 2)
                repetitions++;

            const auto look_behind = look_behind_table.at(idx);
            if (output_ptr + look_behind < output_start)
                throw err::BadDataOffsetError();
            if (output_ptr + repetitions > output_end)
                throw err::BadDataSizeError();

            while (repetitions-- && output_ptr < output_end)
            {
                *output_ptr = output_ptr[look_behind];
                output_ptr++;
            }
        }
    }

    return pix::Grid(width, height, output, palette);
}

static auto dummy = fmt::register_fmt<Ed8ImageDecoder>("active-soft/ed8");
