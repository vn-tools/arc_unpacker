#include "fmt/leaf/leafpack_group/p16_audio_decoder.h"
#include "util/file_from_wave.h"

using namespace au;
using namespace au::fmt::leaf;

bool P16AudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.has_extension("P16");
}

std::unique_ptr<io::File> P16AudioDecoder::decode_impl(
    io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto samples = input_file.stream.read_to_eof();
    sfx::Wave wave;
    wave.data.samples = samples;
    return util::file_from_wave(wave, input_file.name);
}

static auto dummy = fmt::register_fmt<P16AudioDecoder>("leaf/p16");
