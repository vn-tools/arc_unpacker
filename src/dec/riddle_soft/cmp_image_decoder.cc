#include "dec/riddle_soft/cmp_image_decoder.h"
#include "algo/pack/lzss.h"
#include "dec/microsoft/bmp_image_decoder.h"

using namespace au;
using namespace au::dec::riddle_soft;

static const bstr magic = "CMP1"_b;

bool CmpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image CmpImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto size_comp = input_file.stream.read_le<u32>();

    auto data = input_file.stream.read(size_comp);
    algo::pack::BitwiseLzssSettings settings;
    settings.position_bits = 11;
    settings.size_bits = 4;
    settings.min_match_size = 2;
    settings.initial_dictionary_pos = 2031;
    data = algo::pack::lzss_decompress(data, size_orig, settings);

    io::File bmp_file(input_file.path, data);
    const auto bmp_image_decoder = dec::microsoft::BmpImageDecoder();
    return bmp_image_decoder.decode(logger, bmp_file);
}

static auto _ = dec::register_decoder<CmpImageDecoder>("riddle-soft/cmp");
