#include "dec/amuse_craft/bgm_audio_decoder.h"

using namespace au;
using namespace au::dec::amuse_craft;

static const bstr magic = "BGM\x20"_b;

bool BgmAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> BgmAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());

    // If I had to guess, I'd say these contain loop information.
    // In any case, these don't seem too important.
    input_file.stream.skip(8);

    auto output_file = std::make_unique<io::File>();
    output_file->stream.write(input_file.stream.read_to_eof());
    output_file->path = input_file.path;
    output_file->guess_extension();
    return output_file;
}

static auto _ = dec::register_decoder<BgmAudioDecoder>("amuse-craft/bgm");
