// Majiro paletted image
//
// Company:   Majiro
// Engine:    -
// Extension: .rc8
// Archives:  ARC
//
// Known games:
// - [Empress] [150626] Closed Game

#include "fmt/majiro/rc8_converter.h"
#include "io/buffered_io.h"
#include "util/image.h"
#include "util/range.h"
#include "util/require.h"

using namespace au;
using namespace au::fmt::majiro;

static const bstr magic = "\x98\x5A\x92\x9A\x38\x5f\x30\x30"_b;

static bstr uncompress(const bstr &input, size_t width, size_t height)
{
    io::BufferedIO input_io(input);

    bstr output;
    output.resize(width * height);

    auto output_ptr = output.get<u8>();
    auto output_end = output.end<const u8>();

    std::vector<int> shift_table;
    for (auto i : util::range(4))
        shift_table.push_back(-1 - i);
    for (auto i : util::range(7))
        shift_table.push_back(3 - i - width);
    for (auto i : util::range(5))
        shift_table.push_back(2 - i - width * 2);

    util::require(output_ptr < output_end);
    *output_ptr++ = input_io.read_u8();

    while (output_ptr < output_end)
    {
        auto flag = input_io.read_u8();
        if (flag & 0x80)
        {
            auto size = flag & 7;
            auto look_behind = (flag >> 3) & 0x0F;
            size = size == 7
                ? input_io.read_u16_le() + 0x0A
                : size + 3;
            auto source_ptr = &output_ptr[shift_table[look_behind]];
            util::require(source_ptr >= output.get<u8>());
            util::require(source_ptr < output_end);
            while (size-- && output_ptr < output_end)
                *output_ptr++ = *source_ptr++;
        }
        else
        {
            auto size = flag == 0x7F
                ? input_io.read_u16_le() + 0x80
                : flag + 1;
            while (size-- && output_ptr < output_end)
                *output_ptr++ = input_io.read_u8();
        }
    }

    return output;
}

bool Rc8Converter::is_recognized_internal(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> Rc8Converter::decode_internal(File &file) const
{
    file.io.skip(magic.size());
    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    file.io.skip(4);

    pix::Palette palette(256, file.io, pix::Format::BGR888);
    auto data_comp = file.io.read_to_eof();
    auto data_orig = uncompress(data_comp, width, height);
    pix::Grid pixels(width, height, data_orig, palette);
    return util::Image::from_pixels(pixels)->create_file(file.name);
}

static auto dummy = fmt::Registry::add<Rc8Converter>("majiro/rc8");
