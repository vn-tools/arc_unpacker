#include "fmt/bgi/audio_decoder.h"

using namespace au;
using namespace au::fmt::bgi;

static const bstr magic = "bw\x20\x20"_b;

bool AudioDecoder::is_recognized_impl(File &file) const
{
    file.stream.skip(4);
    return file.stream.read(magic.size()) == magic;
}

std::unique_ptr<File> AudioDecoder::decode_impl(File &file) const
{
    file.stream.seek(0);
    const auto header_size = file.stream.read_u32_le();
    file.stream.skip(magic.size());
    const auto file_size = file.stream.read_u32_le();
    file.stream.seek(header_size);
    const auto data = file.stream.read(file_size);

    auto output_file = std::make_unique<File>(file.name, data);
    output_file->change_extension("ogg");
    return output_file;
}

static auto dummy = fmt::register_fmt<AudioDecoder>("bgi/audio");
