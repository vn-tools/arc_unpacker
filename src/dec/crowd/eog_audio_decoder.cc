#include "dec/crowd/eog_audio_decoder.h"
#include "algo/range.h"
#include "err.h"

using namespace au;
using namespace au::dec::crowd;

static const bstr magic = "CRM\x00"_b;

bool EogAudioDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<io::File> EogAudioDecoder::decode_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_size = input_file.stream.read_u32_le();
    const auto ogg_data = input_file.stream.read_to_eof();
    auto output_file = std::make_unique<io::File>(input_file.path, ogg_data);
    output_file->path.change_extension("ogg");
    return output_file;
}

static auto _ = dec::register_decoder<EogAudioDecoder>("crowd/eog");
