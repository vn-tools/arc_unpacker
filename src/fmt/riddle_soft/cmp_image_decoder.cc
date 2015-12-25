#include "fmt/riddle_soft/cmp_image_decoder.h"
#include "algo/pack/lzss.h"
#include "fmt/microsoft/bmp_image_decoder.h"

using namespace au;
using namespace au::fmt::riddle_soft;

static const bstr magic = "CMP1"_b;

bool CmpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image CmpImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    auto size_original = input_file.stream.read_u32_le();
    auto size_compressed = input_file.stream.read_u32_le();

    auto data = input_file.stream.read(size_compressed);
    algo::pack::BitwiseLzssSettings settings;
    settings.position_bits = 11;
    settings.size_bits = 4;
    settings.min_match_size = 2;
    settings.initial_dictionary_pos = 2031;
    data = algo::pack::lzss_decompress(data, size_original, settings);

    io::File bmp_file(input_file.path, data);
    const fmt::microsoft::BmpImageDecoder bmp_image_decoder;
    return bmp_image_decoder.decode(logger, bmp_file);
}

static auto dummy = fmt::register_fmt<CmpImageDecoder>("riddle-soft/cmp");
