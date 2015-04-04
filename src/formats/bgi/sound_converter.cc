// BGI music/sound
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: -
// Archives:  ARC
//
// Known games:
// - Higurashi No Naku Koro Ni

#include "formats/bgi/sound_converter.h"
using namespace Formats::Bgi;

namespace
{
    const std::string magic("bw  ", 4);
}

std::unique_ptr<File> SoundConverter::decode_internal(File &file) const
{
    size_t header_size = file.io.read_u32_le();
    if (file.io.read(magic.length()) != magic)
        throw std::runtime_error("Not a BGI sound");

    uint32_t file_size = file.io.read_u32_le();
    file.io.seek(header_size);
    std::unique_ptr<File> output_file(new File);
    output_file->io.write_from_io(file.io, file_size);
    output_file->name = file.name;
    output_file->change_extension("ogg");
    return output_file;
}
