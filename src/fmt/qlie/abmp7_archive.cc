#include "fmt/qlie/abmp7_archive.h"
#include "util/encoding.h"

using namespace au;
using namespace au::fmt::qlie;

static const std::string magic("ABMP7", 5);

static void read_first_file(io::IO &arc_io, FileSaver &file_saver)
{
    size_t length = arc_io.read_u32_le();
    std::unique_ptr<File> subfile(new File);
    subfile->io.write_from_io(arc_io, length);
    subfile->name = "base.dat";
    subfile->guess_extension();
    file_saver.save(std::move(subfile));
}

static void read_next_file(io::IO &arc_io, FileSaver &file_saver)
{
    std::string encoded_name = arc_io.read(arc_io.read_u8());
    arc_io.skip(31 - encoded_name.size());
    std::string name = util::sjis_to_utf8(encoded_name);
    size_t length = arc_io.read_u32_le();
    std::unique_ptr<File> subfile(new File);
    subfile->io.write_from_io(arc_io, length);
    subfile->name = (name == "" ? "unknown" : name) + ".dat";
    subfile->guess_extension();
    file_saver.save(std::move(subfile));
}

bool Abmp7Archive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void Abmp7Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.seek(12);
    arc_file.io.skip(arc_file.io.read_u32_le());

    read_first_file(arc_file.io, file_saver);
    while (arc_file.io.tell() < arc_file.io.size())
        read_next_file(arc_file.io, file_saver);
}
