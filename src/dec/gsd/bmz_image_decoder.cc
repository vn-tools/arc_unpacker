#include "dec/gsd/bmz_image_decoder.h"
#include "algo/pack/zlib.h"
#include "dec/microsoft/bmp_image_decoder.h"

using namespace au;
using namespace au::dec::gsd;

static const bstr magic = "ZLC3"_b;

bool BmzImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image BmzImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto size_orig = input_file.stream.read_le<u32>();
    const auto data = algo::pack::zlib_inflate(input_file.stream.read_to_eof());
    const auto pseudo_file = std::make_unique<io::File>(input_file.path, data);
    return dec::microsoft::BmpImageDecoder().decode(logger, *pseudo_file);
}

static auto _ = dec::register_decoder<BmzImageDecoder>("gsd/bmz");
