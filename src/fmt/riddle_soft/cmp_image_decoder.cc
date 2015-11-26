#include "fmt/riddle_soft/cmp_image_decoder.h"
#include "fmt/microsoft/bmp_image_decoder.h"
#include "util/pack/lzss.h"

using namespace au;
using namespace au::fmt::riddle_soft;

static const bstr magic = "CMP1"_b;

bool CmpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

pix::Grid CmpImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.skip(magic.size());
    auto size_original = input_file.stream.read_u32_le();
    auto size_compressed = input_file.stream.read_u32_le();

    auto data = input_file.stream.read(size_compressed);
    util::pack::LzssSettings settings;
    settings.position_bits = 11;
    settings.size_bits = 4;
    settings.min_match_size = 2;
    settings.initial_dictionary_pos = 2031;
    data = util::pack::lzss_decompress_bitwise(data, size_original, settings);

    io::File bmp_file(input_file.name, data);
    const fmt::microsoft::BmpImageDecoder bmp_image_decoder;
    return bmp_image_decoder.decode(bmp_file);
}

static auto dummy = fmt::register_fmt<CmpImageDecoder>("riddle-soft/cmp");
