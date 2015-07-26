// ARC archive
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: .arc
//
// Known games:
// - Higurashi No Naku Koro Ni

#include "fmt/bgi/arc_archive.h"
#include "fmt/bgi/cbg_converter.h"
#include "fmt/bgi/sound_converter.h"

using namespace au;
using namespace au::fmt::bgi;

static const std::string magic("PackFile    ", 12);

static std::unique_ptr<File> read_file(io::IO &arc_io, size_t file_count)
{
    std::unique_ptr<File> file(new File);

    size_t old_pos = arc_io.tell();
    file->name = arc_io.read_until_zero();
    arc_io.seek(old_pos + 16);

    size_t offset = arc_io.read_u32_le();
    size_t size = arc_io.read_u32_le();
    offset += magic.size() + 4 + file_count * 32;
    arc_io.skip(8);

    old_pos = arc_io.tell();
    arc_io.seek(offset);
    file->io.write_from_io(arc_io, size);
    arc_io.seek(old_pos);

    return file;
}

struct ArcArchive::Priv
{
    CbgConverter cbg_converter;
    SoundConverter sound_converter;
};

ArcArchive::ArcArchive() : p(new Priv)
{
    add_transformer(&p->cbg_converter);
    add_transformer(&p->sound_converter);
}

ArcArchive::~ArcArchive()
{
}

bool ArcArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void ArcArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());

    size_t file_count = arc_file.io.read_u32_le();
    for (size_t i = 0; i < file_count; i++)
        file_saver.save(read_file(arc_file.io, file_count));
}
