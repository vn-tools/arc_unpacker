#include "dec/aoi/aog_audio_decoder.h"

using namespace au;
using namespace au::dec::aoi;

static const bstr aoi_magic = "AoiOgg"_b;
static const bstr ogg_magic = "OggS"_b;

bool AogAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(aoi_magic.size()) == aoi_magic
        || (input_file.stream.seek(0).read(ogg_magic.size()) == ogg_magic
            && input_file.path.has_extension("aog"));
}

std::unique_ptr<io::File> AogAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    return std::make_unique<io::File>(
        io::path(input_file.path).change_extension("ogg"),
        input_file.stream.seek(0).read(aoi_magic.size()) == aoi_magic
            ? input_file.stream.seek(0x2C).read_to_eof()
            : input_file.stream.seek(0).read_to_eof());
}

static auto _ = dec::register_decoder<AogAudioDecoder>("aoi/aog");
