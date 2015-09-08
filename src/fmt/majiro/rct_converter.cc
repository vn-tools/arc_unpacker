// Majiro true color image
//
// Company:   Majiro
// Engine:    -
// Extension: .rct
// Archives:  ARC
//
// Known games:
// - [Empress] [150626] Closed Game

#include "fmt/majiro/rct_converter.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/image.h"
#include "util/range.h"
#include "util/require.h"

using namespace au;
using namespace au::fmt::majiro;

static const std::vector<bstr> magic =
{
    util::utf8_to_sjis("六丁TC00"_b),
    util::utf8_to_sjis("六丁TC01"_b),
    util::utf8_to_sjis("六丁TS00"_b),
    util::utf8_to_sjis("六丁TS01"_b)
};

static bstr uncompress(const bstr &input, size_t width, size_t height)
{
    io::BufferedIO input_io(input);

    bstr output;
    output.resize(width * height * 3);

    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();

    std::vector<int> shift_table;
    for (auto i : util::range(6))
        shift_table.push_back((-1 - i) * 3);
    for (auto i : util::range(7))
        shift_table.push_back((3 - i - width) * 3);
    for (auto i : util::range(7))
        shift_table.push_back((3 - i - width * 2) * 3);
    for (auto i : util::range(7))
        shift_table.push_back((3 - i - width * 3) * 3);
    for (auto i : util::range(5))
        shift_table.push_back((2 - i - width * 4) * 3);

    util::require(output_ptr + 3 <= output_end);
    *output_ptr++ = input_io.read_u8();
    *output_ptr++ = input_io.read_u8();
    *output_ptr++ = input_io.read_u8();

    while (output_ptr < output_end)
    {
        auto flag = input_io.read_u8();
        if (flag & 0x80)
        {
            auto size = flag & 3;
            auto look_behind = (flag >> 2) & 0x1F;
            size = size == 3
                ? (input_io.read_u16_le() + 4) * 3
                : size * 3 + 3;
            auto source_ptr = &output_ptr[shift_table[look_behind]];
            util::require(source_ptr >= output.get<u8>());
            util::require(source_ptr < output_end);
            while (size-- && output_ptr < output_end)
                *output_ptr++ = *source_ptr++;
        }
        else
        {
            auto size = flag == 0x7F
                ? (input_io.read_u16_le() + 0x80) * 3
                : flag * 3 + 3;
            while (size-- && output_ptr < output_end)
                *output_ptr++ = input_io.read_u8();
        }
    }

    return output;
}

bool RctConverter::is_recognized_internal(File &file) const
{
    for (const auto &m : magic)
    {
        file.io.seek(0);
        if (file.io.read(m.size()) == m)
            return true;
    }
    return false;
}

std::unique_ptr<File> RctConverter::decode_internal(File &file) const
{
    for (const auto &m : magic)
    {
        file.io.seek(0);
        if (file.io.read(m.size()) == m)
            break;
    }

    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    file.io.skip(4);
    auto data_comp = file.io.read_to_eof();
    auto data_orig = uncompress(data_comp, width, height);
    pix::Grid pixels(width, height, data_orig, pix::Format::BGR888);
    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<RctConverter>("majiro/rct");
