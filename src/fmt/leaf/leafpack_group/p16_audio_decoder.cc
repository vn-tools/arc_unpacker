#include "fmt/leaf/leafpack_group/p16_audio_decoder.h"
#include "util/file_from_wave.h"

using namespace au;
using namespace au::fmt::leaf;

bool P16AudioDecoder::is_recognized_impl(File &file) const
{
    return file.has_extension("P16");
}

std::unique_ptr<File> P16AudioDecoder::decode_impl(File &file) const
{
    file.stream.seek(0);
    const auto samples = file.stream.read_to_eof();
    sfx::Wave wave;
    wave.data.samples = samples;
    return util::file_from_wave(wave, file.name);
}

static auto dummy = fmt::register_fmt<P16AudioDecoder>("leaf/p16");
