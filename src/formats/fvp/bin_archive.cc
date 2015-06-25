// FVP archive
//
// Company:   Favorite
// Engine:    Favorite View Point
// Extension: .bin
//
// Known games:
// - Irotoridori no Sekai

#include "formats/fvp/bin_archive.h"
#include "formats/fvp/nvsg_converter.h"
#include "util/encoding.h"
using namespace Formats::Fvp;

namespace
{
    typedef struct
    {
        std::string name;
        u32 offset;
        u32 size;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    Table read_table(IO &arc_io)
    {
        size_t file_count = arc_io.read_u32_le();
        arc_io.skip(4);

        size_t names_start = file_count * 12 + 8;

        Table table;
        for (size_t i = 0; i < file_count; i++)
        {
            std::unique_ptr<TableEntry> entry(new TableEntry);
            size_t name_offset = arc_io.read_u32_le();
            arc_io.peek(names_start + name_offset, [&]()
            {
                entry->name = convert_encoding(
                    arc_io.read_until_zero(), "cp932", "utf-8");
            });
            entry->offset = arc_io.read_u32_le();
            entry->size = arc_io.read_u32_le();
            table.push_back(std::move(entry));
        }
        return table;
    }

    std::unique_ptr<File> read_file(IO &arc_io, TableEntry &entry)
    {
        std::unique_ptr<File> file(new File);
        arc_io.seek(entry.offset);
        file->io.write_from_io(arc_io, entry.size);
        file->name = entry.name;
        return file;
    }
}

struct BinArchive::Priv
{
    NvsgConverter nvsg_converter;
};

BinArchive::BinArchive() : p(new Priv)
{
    add_transformer(&p->nvsg_converter);
}

BinArchive::~BinArchive()
{
}

bool BinArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("bin");
}

void BinArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}
