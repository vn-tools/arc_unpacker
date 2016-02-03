#include "dec/kaguya/ap2_image_decoder.h"
#include "err.h"

using namespace au;
using namespace au::dec::kaguya;

static const bstr magic = "AP-2"_b;

bool Ap2ImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

res::Image Ap2ImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto x = input_file.stream.read_le<u32>();
    const auto y = input_file.stream.read_le<u32>();
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    const auto depth = input_file.stream.read_le<u32>();
    if (depth != 24)
        throw err::UnsupportedBitDepthError(depth);
    const auto data = input_file.stream.read_to_eof();
    return res::Image(width, height, data, res::PixelFormat::BGRA8888)
        .flip_vertically();
}

static auto _ = dec::register_decoder<Ap2ImageDecoder>("kaguya/ap2");
