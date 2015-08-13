// BGI music/sound
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: -
// Archives:  ARC
//
// Known games:
// - Higurashi No Naku Koro Ni
// - Go! Go! Nippon! ~My First Trip to Japan~

#include "fmt/bgi/sound_converter.h"

using namespace au;
using namespace au::fmt::bgi;

static const bstr magic = "bw\x20\x20"_b;

bool SoundConverter::is_recognized_internal(File &file) const
{
    file.io.skip(4);
    return file.io.read(magic.size()) == magic;
}

std::unique_ptr<File> SoundConverter::decode_internal(File &file) const
{
    size_t header_size = file.io.read_u32_le();
    file.io.skip(magic.size());

    u32 file_size = file.io.read_u32_le();
    file.io.seek(header_size);
    std::unique_ptr<File> output_file(new File);
    output_file->io.write_from_io(file.io, file_size);
    output_file->name = file.name;
    output_file->change_extension("ogg");
    return output_file;
}
