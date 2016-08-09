#include "dec/almond_collective/teyl_image_decoder.h"

using namespace au;
using namespace au::dec::almond_collective;

static const auto magic = "TEYL"_b;

bool TeylImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

res::Image TeylImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto width = input_file.stream.read_le<u32>();
    const auto height = input_file.stream.read_le<u32>();
    return res::Image(
        width, height, input_file.stream, res::PixelFormat::BGRA8888);
}

static auto _
    = dec::register_decoder<TeylImageDecoder>("almond-collective/teyl");
