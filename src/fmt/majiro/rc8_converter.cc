// Majiro paletted image
//
// Company:   Majiro
// Engine:    -
// Extension: .rc8
// Archives:  ARC
//
// Known games:
// - [Empress] [150626] Closed Game

#include "err.h"
#include "fmt/majiro/rc8_converter.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/image.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::majiro;

static const bstr magic = util::utf8_to_sjis("六丁8"_b);

static bstr uncompress(const bstr &input, size_t width, size_t height)
{
    io::BufferedIO input_io(input);

    bstr output(width * height);
    auto output_ptr = output.get<u8>();
    auto output_start = output.get<const u8>();
    auto output_end = output.end<const u8>();

    std::vector<int> shift_table;
    for (auto i : util::range(4))
        shift_table.push_back(-1 - i);
    for (auto i : util::range(7))
        shift_table.push_back(3 - i - width);
    for (auto i : util::range(5))
        shift_table.push_back(2 - i - width * 2);

    if (output.size() < 1)
        return output;
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
            if (source_ptr < output_start || source_ptr + size >= output_end)
                return output;
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

    if (file.io.read_u8() != '_')
        throw err::NotSupportedError("Unexpected encryption flag");

    int version = std::stoi(file.io.read(2).str());
    if (version != 0)
        throw err::UnsupportedVersionError(version);

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
