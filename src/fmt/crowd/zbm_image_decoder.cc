#include "fmt/crowd/zbm_image_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "fmt/microsoft/bmp_image_decoder.h"

using namespace au;
using namespace au::fmt::crowd;

static const auto magic = "SZDD"_b;

bool ZbmImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image ZbmImageDecoder::decode_impl(io::File &input_file) const
{
    input_file.stream.seek(10);
    const auto size_orig = input_file.stream.read_u32_le();

    algo::pack::BytewiseLzssSettings settings;
    settings.initial_dictionary_pos = 0xFF0;
    auto data = algo::pack::lzss_decompress(
        input_file.stream.read_to_eof(), size_orig, settings);
    for (const auto i : algo::range(std::min<size_t>(100, data.size())))
        data[i] ^= 0xFF;

    const fmt::microsoft::BmpImageDecoder bmp_decoder;
    io::File bmp_file("dummy.bmp", data);
    return bmp_decoder.decode(bmp_file);
}

static auto dummy = fmt::register_fmt<ZbmImageDecoder>("crowd/zbm");
