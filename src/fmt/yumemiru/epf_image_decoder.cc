#include "fmt/yumemiru/epf_image_decoder.h"
#include "algo/pack/lzss.h"
#include "err.h"

using namespace au;
using namespace au::fmt::yumemiru;

static const bstr magic = "yga\x00"_b;

bool EpfImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image EpfImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto width = input_file.stream.read_u32_le();
    const auto height = input_file.stream.read_u32_le();
    if (input_file.stream.read_u32_le() != 1)
        throw err::CorruptDataError("Expected '1'");
    const auto size_orig = input_file.stream.read_u32_le();
    const auto size_comp = input_file.stream.read_u32_le();
    const auto data = algo::pack::lzss_decompress_bytewise(
        input_file.stream.read(size_comp), size_orig);
    return res::Image(width, height, data, res::PixelFormat::BGRA8888);
}

static auto dummy = fmt::register_fmt<EpfImageDecoder>("yumemiru/epf");
