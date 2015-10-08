#include "fmt/bgi/audio_decoder.h"

using namespace au;
using namespace au::fmt::bgi;

static const bstr magic = "bw\x20\x20"_b;

bool AudioDecoder::is_recognized_internal(File &file) const
{
    file.io.skip(4);
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> AudioDecoder::decode_internal(File &file) const
{
    size_t header_size = file.io.read_u32_le();
    file.io.skip(magic.size());

    u32 file_size = file.io.read_u32_le();
    file.io.seek(header_size);
    auto output_file = std::make_unique<File>();
    output_file->io.write_from_io(file.io, file_size);
    output_file->name = file.name;
    output_file->change_extension("ogg");
    return output_file;
}

static auto dummy = fmt::Registry::add<AudioDecoder>("bgi/sound");
