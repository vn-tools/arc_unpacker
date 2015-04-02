// ARC archive
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: .arc
//
// Known games:
// - Higurashi No Naku Koro Ni

#include "formats/bgi/arc_archive.h"
#include "formats/bgi/cbg_converter.h"
#include "formats/bgi/sound_converter.h"
using namespace Formats::Bgi;

namespace
{
    const std::string magic("PackFile    ", 12);

    std::unique_ptr<File> read_file(IO &arc_io, size_t file_count)
    {
        std::unique_ptr<File> file(new File);

        size_t old_pos = arc_io.tell();
        file->name = arc_io.read_until_zero();
        arc_io.seek(old_pos + 16);

        size_t offset = arc_io.read_u32_le();
        size_t size = arc_io.read_u32_le();
        offset += magic.size() + 4 + file_count * 32;
        arc_io.skip(8);
        if (offset + size > arc_io.size())
            throw std::runtime_error("Bad offset to file");

        old_pos = arc_io.tell();
        arc_io.seek(offset);
        file->io.write_from_io(arc_io, size);
        arc_io.seek(old_pos);

        return file;
    }
}

struct ArcArchive::Internals
{
    CbgConverter cbg_converter;
    SoundConverter sound_converter;
};

ArcArchive::ArcArchive() : internals(new Internals)
{
    add_transformer(&internals->cbg_converter);
    add_transformer(&internals->sound_converter);
}

ArcArchive::~ArcArchive()
{
}

void ArcArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    if (arc_file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not an ARC archive");

    size_t file_count = arc_file.io.read_u32_le();
    if (file_count * 32 > arc_file.io.size())
        throw std::runtime_error("Bad file count");

    for (size_t i = 0; i < file_count; i ++)
        file_saver.save(read_file(arc_file.io, file_count));
}
