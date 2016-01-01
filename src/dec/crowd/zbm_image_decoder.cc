#include "dec/crowd/zbm_image_decoder.h"
#include "algo/pack/lzss.h"
#include "algo/range.h"
#include "dec/microsoft/bmp_image_decoder.h"

using namespace au;
using namespace au::dec::crowd;

static const auto magic = "SZDD"_b;

bool ZbmImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic
        && input_file.path.has_extension("zbm");
}

res::Image ZbmImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(10);
    const auto size_orig = input_file.stream.read_u32_le();

    algo::pack::BytewiseLzssSettings settings;
    settings.initial_dictionary_pos = 0xFF0;
    auto data = algo::pack::lzss_decompress(
        input_file.stream.read_to_eof(), size_orig, settings);
    for (const auto i : algo::range(std::min<size_t>(100, data.size())))
        data[i] ^= 0xFF;

    const auto bmp_decoder =  dec::microsoft::BmpImageDecoder();
    io::File bmp_file("dummy.bmp", data);
    return bmp_decoder.decode(logger, bmp_file);
}

static auto _ = dec::register_decoder<ZbmImageDecoder>("crowd/zbm");
