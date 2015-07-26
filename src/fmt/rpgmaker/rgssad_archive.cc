// RGSSAD archive
//
// Company:   Enterbrain
// Engine:    Ruby Game Scripting System (RPG Maker XP)
// Extension: .rgssad
//
// Known games:
// - Cherry Tree High Comedy Club

#include "fmt/rpgmaker/rgssad_archive.h"

using namespace au;
using namespace au::fmt::rpgmaker;

namespace
{
    typedef struct
    {
        std::string name;
        size_t size;
        size_t offset;
        u32 key;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;
}

static const std::string magic("RGSSAD\x00", 7);
static const u32 initial_key = 0xdeadcafe;

static u32 advance_key(const u32 key)
{
    return key * 7 + 3;
}

static Table read_table(io::IO &arc_io, u32 key)
{
    Table table;
    while (arc_io.tell() < arc_io.size())
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);

        size_t name_length = arc_io.read_u32_le() ^ key;
        key = advance_key(key);
        entry->name = arc_io.read(name_length);
        for (size_t i = 0; i < name_length; i++)
        {
            entry->name[i] ^= key;
            key = advance_key(key);
        }

        entry->size = arc_io.read_u32_le() ^ key;
        key = advance_key(key);

        entry->key = key;
        entry->offset = arc_io.tell();

        arc_io.skip(entry->size);
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;
    arc_io.seek(entry.offset);
    file->io.write_from_io(arc_io, entry.size);

    file->io.write("\x00\x00\x00\x00", 4);
    file->io.seek(0);
    u32 key = entry.key;
    for (size_t i = 0; i + 4 < file->io.size(); i += 4)
    {
        u32 chunk = file->io.read_u32_le();
        chunk ^= key;
        key = advance_key(key);
        file->io.skip(-4);
        file->io.write_u32_le(chunk);
    }
    file->io.truncate(entry.size);
    return file;
}

bool RgssadArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void RgssadArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());

    u8 version = arc_file.io.read_u8();
    if (version != 1)
        throw std::runtime_error("Unsupported archive version");

    u32 key = initial_key;

    Table table = read_table(arc_file.io, key);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}
