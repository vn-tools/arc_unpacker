#include "fmt/leaf/pak2_group/pak2_compressed_file_decoder.h"
#include "io/memory_stream.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "\xAF\xF6\x4D\x4F"_b;

static bstr decompress(const bstr &src, const size_t size_orig)
{
    bstr output;
    output.reserve(size_orig);
    io::MemoryStream input_stream(src);
    while (output.size() < size_orig && !input_stream.eof())
    {
        const auto control = input_stream.read_u8();
        if (control >= 0x80)
        {
            if (control >= 0xA0)
            {
                int repeat;
                int base_value;
                if (control == 0xFF)
                {
                    repeat = input_stream.read_u8() + 2;
                    base_value = 0;
                }
                else if (control >= 0xE0)
                {
                    repeat = (control & 0x1F) + 2;
                    base_value = 0;
                }
                else
                {
                    repeat = (control & 0x1F) + 2;
                    base_value = input_stream.read_u8();
                }
                output += bstr(repeat, base_value);
            }
            else
            {
                const auto repeat = control & 0x1F;
                output += input_stream.read(repeat);
            }
        }
        else
        {
            const auto look_behind
                = (input_stream.read_u8() + (control << 8)) % 0x400;
            const auto repeat = (control >> 2) + 2;
            for (const auto i : util::range(repeat))
                output += output[output.size() - look_behind];
        }
    }
    return output;
}

bool Pak2CompressedFileDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(4).read(magic.size()) == magic;
}

std::unique_ptr<io::File> Pak2CompressedFileDecoder::decode_impl(
    io::File &input_file) const
{
    const auto size_orig = input_file.stream.seek(24).read_u32_le();
    const auto data = input_file.stream.seek(36).read_to_eof();
    return std::make_unique<io::File>(
        input_file.name, decompress(data, size_orig));
}

static auto dummy
    = fmt::register_fmt<Pak2CompressedFileDecoder>("leaf/pak2-compressed-file");
