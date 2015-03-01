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
    const uint32_t initial_key = 0xdeadcafe;

    uint32_t advance_key(const uint32_t key)
    {
        return key * 7 + 3;
    }

    typedef struct
    {
        std::string name;
        size_t size;
        size_t offset;
        uint32_t key;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    Table read_table(IO &arc_io, uint32_t key)
    {
        Table table;
        while (arc_io.tell() < arc_io.size())
        {
            std::unique_ptr<TableEntry> table_entry(new TableEntry);

            size_t name_length = arc_io.read_u32_le() ^ key;
            key = advance_key(key);
            table_entry->name = arc_io.read(name_length);
            for (size_t i = 0; i < name_length; i ++)
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
        uint32_t key = table_entry.key;
        for (size_t i = 0; i + 4 < file->io.size(); i += 4)
        {
            uint32_t chunk = file->io.read_u32_le();
            chunk ^= key;
            key = advance_key(key);
            file->io.skip(-4);
            file->io.write_u32_le(chunk);
        }
        file->io.truncate(table_entry.size);
        return file;
    }
}

void RgssadArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    if (arc_file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a RGSSAD archive");

    uint8_t version = arc_file.io.read_u8();
    if (version != 1)
        throw std::runtime_error("Unsupported archive version");

    uint32_t key = initial_key;

    Table table = read_table(arc_file.io, key);
    for (auto &table_entry : table)
        file_saver.save(read_file(arc_file.io, *table_entry));
}
