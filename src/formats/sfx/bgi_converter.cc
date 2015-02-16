// BGI music/sound
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: -
// Archives:  ARC
//
// Known games:
// - Higurashi No Naku Koro Ni

#include "formats/sfx/bgi_converter.h"

namespace
{
    const std::string magic("bw  ", 4);
}

void BgiConverter::decode_internal(File &file) const
{
    size_t header_size = file.io.read_u32_le();
    if (file.io.read(magic.length()) != magic)
        throw std::runtime_error("Not a BGI sound");

    uint32_t file_size = file.io.read_u32_le();
    file.io.seek(header_size);
    std::string data = file.io.read(file_size);
    file.io.truncate(0);
    file.io.write(data);
    file.change_extension("ogg");
}
