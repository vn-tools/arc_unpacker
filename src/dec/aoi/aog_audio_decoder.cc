#include "dec/aoi/aog_audio_decoder.h"

using namespace au;
using namespace au::dec::aoi;

static const bstr magic = "AoiOgg"_b;

bool AogAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> AogAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    return std::make_unique<io::File>(
        io::path(input_file.path).change_extension("ogg"),
        input_file.stream.seek(0x2C).read_to_eof());
}

static auto _ = dec::register_decoder<AogAudioDecoder>("aoi/aog");
