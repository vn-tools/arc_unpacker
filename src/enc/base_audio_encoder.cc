#include "enc/base_audio_encoder.h"

using namespace au;
using namespace au::enc;

std::unique_ptr<io::File> BaseAudioEncoder::encode(
    const Logger &logger,
    const res::Audio &input_audio,
    const io::path &name) const
{
    auto output_file = std::make_unique<io::File>(name, ""_b);
    encode_impl(logger, input_audio, *output_file);
    return output_file;
}
