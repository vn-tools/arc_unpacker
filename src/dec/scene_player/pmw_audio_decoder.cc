#include "dec/scene_player/pmw_audio_decoder.h"
#include "algo/binary.h"
#include "algo/pack/zlib.h"

using namespace au;
using namespace au::dec::scene_player;

bool PmwAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("pmw");
}

std::unique_ptr<io::File> PmwAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto data
        = algo::pack::zlib_inflate(
            algo::unxor(
                input_file.stream.seek(0).read_to_eof(), 0x21));
    auto output_file = std::make_unique<io::File>(input_file.path, data);
    output_file->path.change_extension("wav");
    return output_file;
}

static auto _ = dec::register_decoder<PmwAudioDecoder>("scene-player/pwm");
