#include "dec/scene_player/pmp_image_decoder.h"
#include "algo/binary.h"
#include "algo/pack/zlib.h"
#include "dec/microsoft/bmp_image_decoder.h"

using namespace au;
using namespace au::dec::scene_player;

bool PmpImageDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("pmp");
}

res::Image PmpImageDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto data = algo::pack::zlib_inflate(algo::unxor(
            input_file.stream.seek(0).read_to_eof(), 0x21));
    const auto pseudo_file = std::make_unique<io::File>("dummy.bmp", data);
    const auto bmp_decoder = dec::microsoft::BmpImageDecoder();
    return bmp_decoder.decode(logger, *pseudo_file);
}

static auto _
    = dec::register_decoder<PmpImageDecoder>("scene-player/pmp");
