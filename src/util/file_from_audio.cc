#include "util/file_from_audio.h"
#include "enc/microsoft/wav_audio_encoder.h"

using namespace au;
using namespace au::util;

std::unique_ptr<io::File> util::file_from_audio(
    const res::Audio &audio, const io::path &path)
{
    Logger dummy_logger;
    const auto wav_audio_encoder = enc::microsoft::WavAudioEncoder();
    return wav_audio_encoder.encode(dummy_logger, audio, path);
}
