// XFL archive
//
// Company:   Liar-soft
// Engine:    -
// Extension: .xfl
//
// Known games:
// - Souten No Celenaria - What a Beautiful World
// - Sekien no Inganock - What a Beautiful People
// - Shikkoku no Sharnoth - What a Beautiful Tomorrow

#include "formats/liarsoft/lwg_archive.h"
#include "formats/liarsoft/wcg_converter.h"
#include "formats/liarsoft/xfl_archive.h"
#include "util/encoding.h"
using namespace Formats::LiarSoft;

namespace
{
    typedef struct
    {
        std::string name;
        u32 offset;
        u32 size;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;
}

static const std::string magic("LB\x01\x00", 4);

static Table read_table(IO &arc_io)
{
    Table table;
    size_t table_size = arc_io.read_u32_le();
    size_t file_count = arc_io.read_u32_le();
    size_t file_start = arc_io.tell() + table_size;
    table.reserve(file_count);
    for (size_t i = 0; i < file_count; i++)
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = sjis_to_utf8(arc_io.read_until_zero(0x20));
        entry->offset = file_start + arc_io.read_u32_le();
        entry->size = arc_io.read_u32_le();
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;
    arc_io.seek(entry.offset);
    file->io.write_from_io(arc_io, entry.size);
    return file;
}

struct XflArchive::Priv
{
    LwgArchive lwg_archive;
    WcgConverter wcg_converter;
};

XflArchive::XflArchive() : p(new Priv)
{
    add_transformer(&p->wcg_converter);
    add_transformer(&p->lwg_archive);
    add_transformer(this);
}

XflArchive::~XflArchive()
{
}

bool XflArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void XflArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());

    Table table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}
