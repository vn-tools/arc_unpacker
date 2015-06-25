// MBL archive
//
// Company:   Ivory
// Engine:    MarbleEngine
// Extension: .mbl
//
// Known games:
// - Wanko to Kurasou

#include "formats/ivory/mbl_archive.h"
#include "formats/ivory/prs_converter.h"
#include "util/encoding.h"
using namespace Formats::Ivory;

static int check_version(
    IO &arc_io, size_t initial_position, u32 file_count, u32 name_length)
{
    arc_io.seek(initial_position + file_count * (name_length + 8));
    arc_io.skip(-8);
    u32 last_file_offset = arc_io.read_u32_le();
    u32 last_file_size = arc_io.read_u32_le();
    return last_file_offset + last_file_size == arc_io.size();
}

static int get_version(IO &arc_io)
{
    u32 file_count = arc_io.read_u32_le();
    if (check_version(arc_io, 4, file_count, 16))
    {
        arc_io.seek(0);
        return 1;
    }

    arc_io.seek(4);
    u32 name_length = arc_io.read_u32_le();
    if (check_version(arc_io, 8, file_count, name_length))
    {
        arc_io.seek(0);
        return 2;
    }

    return -1;
}

static std::unique_ptr<File> read_file(IO &arc_io, size_t name_length)
{
    std::unique_ptr<File> file(new File);

    size_t old_pos = arc_io.tell();
    std::string name = arc_io.read_until_zero();
    file->name = sjis_to_utf8(name);
    arc_io.seek(old_pos + name_length);

    size_t offset = arc_io.read_u32_le();
    size_t size = arc_io.read_u32_le();

    old_pos = arc_io.tell();
    arc_io.seek(offset);
    file->io.write_from_io(arc_io, size);
    arc_io.seek(old_pos);

    return file;
}

struct MblArchive::Priv
{
    PrsConverter prs_converter;
};

MblArchive::MblArchive() : p(new Priv)
{
    add_transformer(&p->prs_converter);
}

MblArchive::~MblArchive()
{
}

bool MblArchive::is_recognized_internal(File &arc_file) const
{
    return get_version(arc_file.io) != -1;
}

void MblArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    int version = get_version(arc_file.io);
    u32 file_count = arc_file.io.read_u32_le();
    u32 name_length = version == 2 ? arc_file.io.read_u32_le() : 16;

    for (size_t i = 0; i < file_count; i++)
    {
        auto file = read_file(arc_file.io, name_length);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}
