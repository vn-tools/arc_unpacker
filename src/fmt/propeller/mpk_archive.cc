// MPK archive
//
// Company:   Propeller
// Engine:    -
// Extension: .mpk
//
// Known games:
// - Sukimazakura to Uso no Machi

#include "fmt/propeller/mpk_archive.h"
#include "fmt/propeller/mgr_archive.h"
#include "util/encoding.h"

using namespace au;
using namespace au::fmt::propeller;

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

static Table read_table(io::IO &arc_io)
{
    Table table;
    size_t table_offset = arc_io.read_u32_le();
    size_t file_count = arc_io.read_u32_le();
    table.reserve(file_count);

    arc_io.seek(table_offset);
    for (size_t i = 0; i < file_count; i++)
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = arc_io.read(32);
        entry->offset = arc_io.read_u32_le();
        entry->size = arc_io.read_u32_le();
        table.push_back(std::move(entry));
    }

    if (file_count == 0)
        return table;
    u8 key8 = table[0]->name[31];
    u32 key32 = (key8 << 24) | (key8 << 16) | (key8 << 8) | key8;

    for (auto &entry : table)
    {
        for (size_t i = 0; i < 32; i++)
            entry->name[i] ^= key8;
        entry->name = util::sjis_to_utf8(entry->name);
        if (entry->name[0] == '\\')
            entry->name = entry->name.substr(1);
        entry->name.erase(entry->name.find('\x00'));
        entry->offset ^= key32;
        entry->size ^= key32;
    }

    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;
    arc_io.seek(entry.offset);
    file->io.write_from_io(arc_io, entry.size);
    return file;
}

struct MpkArchive::Priv
{
    MgrArchive mgr_archive;
};

MpkArchive::MpkArchive() : p(new Priv)
{
    add_transformer(&p->mgr_archive);
}

MpkArchive::~MpkArchive()
{
}

bool MpkArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("mpk");
}

void MpkArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}
