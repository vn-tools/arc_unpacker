#include "fmt/audio_decoder.h"
#include "err.h"
#include "util/file_from_wave.h"

using namespace au;
using namespace au::fmt;

AudioDecoder::~AudioDecoder()
{
}

IDecoder::NamingStrategy AudioDecoder::naming_strategy() const
{
    return NamingStrategy::Sibling;
}

void AudioDecoder::unpack(
    io::File &input_file, const FileSaver &file_saver) const
{
    auto output_audio = decode(input_file);
    auto output_file = util::file_from_wave(output_audio, input_file.name);
    // discard any directory information
    output_file->name = io::path(output_file->name).name();
    file_saver.save(std::move(output_file));
}

sfx::Wave AudioDecoder::decode(io::File &file) const
{
    if (!is_recognized(file))
        throw err::RecognitionError();
    file.stream.seek(0);
    return decode_impl(file);
}
