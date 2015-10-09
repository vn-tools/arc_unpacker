#include "fmt/majiro/rct_image_decoder.h"
#include <boost/lexical_cast.hpp>
#include "err.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::majiro;

static const bstr magic = util::utf8_to_sjis("六丁T"_b);

static bstr uncompress(const bstr &input, size_t width, size_t height)
{
    io::BufferedIO input_io(input);

    bstr output(width * height * 3);

    auto output_ptr = output.get<u8>();
    auto output_start = output.get<const u8>();
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

    if (output.size() < 3)
        return output;
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
            if (source_ptr < output_start || source_ptr + size >= output_end)
                return output;
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

bool RctImageDecoder::is_recognized_impl(File &file) const
{
    return file.io.read(magic.size()) == magic;
}

pix::Grid RctImageDecoder::decode_impl(File &file) const
{
    file.io.skip(magic.size());

    bool encrypted;
    auto tmp = file.io.read_u8();
    if (tmp == 'S')
        encrypted = true;
    else if (tmp == 'C')
        encrypted = false;
    else
        throw err::NotSupportedError("Unexpected encryption flag");

    if (encrypted)
        throw err::NotSupportedError("Encrypted RCT images are not supported");

    int version = boost::lexical_cast<int>(file.io.read(2).str());
    if (version < 0 || version > 1)
        throw err::UnsupportedVersionError(version);

    auto width = file.io.read_u32_le();
    auto height = file.io.read_u32_le();
    auto data_size = file.io.read_u32_le();

    std::string base_file;
    if (version == 1)
        base_file = file.io.read(file.io.read_u16_le()).str();

    auto data_comp = file.io.read(data_size);
    auto data_orig = uncompress(data_comp, width, height);
    pix::Grid pixels(width, height, data_orig, pix::Format::BGR888);

    if (version == 1)
    {
        for (auto y : util::range(height))
        for (auto x : util::range(width))
        {
            auto &p = pixels.at(x, y);
            if (p.r == 0xFF && p.g == 0 && p.g == 0)
                p.a = p.r = 0;
        }
    }

    return pixels;
}

static auto dummy = fmt::register_fmt<RctImageDecoder>("majiro/rct");
