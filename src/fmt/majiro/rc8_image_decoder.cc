#include "fmt/majiro/rc8_image_decoder.h"
#include <boost/lexical_cast.hpp>
#include "err.h"
#include "io/memory_stream.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::majiro;

static const bstr magic = util::utf8_to_sjis("六丁8"_b);

static bstr uncompress(const bstr &input, size_t width, size_t height)
{
    io::MemoryStream input_stream(input);

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
    *output_ptr++ = input_stream.read_u8();

    while (output_ptr < output_end)
    {
        auto flag = input_stream.read_u8();
        if (flag & 0x80)
        {
            auto size = flag & 7;
            auto look_behind = (flag >> 3) & 0x0F;
            size = size == 7
                ? input_stream.read_u16_le() + 0x0A
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
                ? input_stream.read_u16_le() + 0x80
                : flag + 1;
            while (size-- && output_ptr < output_end)
                *output_ptr++ = input_stream.read_u8();
        }
    }

    return output;
}

bool Rc8ImageDecoder::is_recognized_impl(File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

pix::Grid Rc8ImageDecoder::decode_impl(File &input_file) const
{
    input_file.stream.skip(magic.size());

    if (input_file.stream.read_u8() != '_')
        throw err::NotSupportedError("Unexpected encryption flag");

    int version = boost::lexical_cast<int>(input_file.stream.read(2).str());
    if (version != 0)
        throw err::UnsupportedVersionError(version);

    auto width = input_file.stream.read_u32_le();
    auto height = input_file.stream.read_u32_le();
    input_file.stream.skip(4);

    pix::Palette palette(256, input_file.stream, pix::Format::BGR888);
    auto data_comp = input_file.stream.read_to_eof();
    auto data_orig = uncompress(data_comp, width, height);
    return pix::Grid(width, height, data_orig, palette);
}

static auto dummy = fmt::register_fmt<Rc8ImageDecoder>("majiro/rc8");
