#include "fmt/bgi/audio_decoder.h"

using namespace au;
using namespace au::fmt::bgi;

static const bstr magic = "bw\x20\x20"_b;

bool AudioDecoder::is_recognized_impl(File &input_file) const
{
    input_file.stream.skip(4);
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<File> AudioDecoder::decode_impl(File &input_file) const
{
    input_file.stream.seek(0);
    const auto header_size = input_file.stream.read_u32_le();
    input_file.stream.skip(magic.size());
    const auto input_file_size = input_file.stream.read_u32_le();
    input_file.stream.seek(header_size);
    const auto data = input_file.stream.read(input_file_size);

    auto output_file = std::make_unique<File>(input_file.name, data);
    output_file->change_extension("ogg");
    return output_file;
}

static auto dummy = fmt::register_fmt<AudioDecoder>("bgi/audio");
