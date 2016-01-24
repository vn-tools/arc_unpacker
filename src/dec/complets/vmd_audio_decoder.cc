#include "dec/complets/vmd_audio_decoder.h"
#include "algo/binary.h"

using namespace au;
using namespace au::dec::complets;

static const auto magic = "\xFF\xFB"_b;

bool VmdAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return algo::unxor(input_file.stream.read(magic.size()), 0xE5) == magic;
}

std::unique_ptr<io::File> VmdAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    return std::make_unique<io::File>(
        io::path(input_file.path).change_extension("mp3"),
        algo::unxor(input_file.stream.seek(0).read_to_eof(), 0xE5));
}

static auto _ = dec::register_decoder<VmdAudioDecoder>("complets/vmd");
