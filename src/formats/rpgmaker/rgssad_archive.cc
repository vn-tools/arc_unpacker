// RGSSAD archive
//
// Company:   Enterbrain
// Engine:    Ruby Game Scripting System (RPG Maker XP)
// Extension: .rgssad
//
// Known games:
// - Cherry Tree High Comedy Club

#include "formats/rpgmaker/rgssad_archive.h"
using namespace Formats::RpgMaker;

namespace
{
    const std::string magic("RGSSAD\x00", 7);
    const u32 initial_key = 0xdeadcafe;

    u32 advance_key(const u32 key)
    {
        return key * 7 + 3;
    }

    typedef struct
    {
        std::string name;
        size_t size;
        size_t offset;
        u32 key;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    Table read_table(IO &arc_io, u32 key)
    {
        Table table;
        while (arc_io.tell() < arc_io.size())
        {
            std::unique_ptr<TableEntry> table_entry(new TableEntry);

            size_t name_length = arc_io.read_u32_le() ^ key;
            key = advance_key(key);
            table_entry->name = arc_io.read(name_length);
            for (size_t i = 0; i < name_length; i++)
            {
                table_entry->name[i] ^= key;
                key = advance_key(key);
            }

            table_entry->size = arc_io.read_u32_le() ^ key;
            key = advance_key(key);

            table_entry->key = key;
            table_entry->offset = arc_io.tell();

            arc_io.skip(table_entry->size);
            table.push_back(std::move(table_entry));
        }
        return table;
    }

    std::unique_ptr<File> read_file(IO &arc_io, TableEntry &table_entry)
    {
        std::unique_ptr<File> file(new File);
        file->name = table_entry.name;
        arc_io.seek(table_entry.offset);
        file->io.write_from_io(arc_io, table_entry.size);

        file->io.write("\x00\x00\x00\x00", 4);
        file->io.seek(0);
        u32 key = table_entry.key;
        for (size_t i = 0; i + 4 < file->io.size(); i += 4)
        {
            u32 chunk = file->io.read_u32_le();
            chunk ^= key;
            key = advance_key(key);
            file->io.skip(-4);
            file->io.write_u32_le(chunk);
        }
        file->io.truncate(table_entry.size);
        return file;
    }
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
    for (auto &table_entry : table)
        file_saver.save(read_file(arc_file.io, *table_entry));
}
