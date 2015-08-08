// aLK archive
//
// Company:   Alice Soft
// Engine:    -
// Extension: .alk
//
// Known games:
// - Daiakuji

#include "fmt/alice_soft/qnt_converter.h"
#include "fmt/alice_soft/alk_archive.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

namespace
{
    struct TableEntry
    {
        std::string name;
        u32 offset;
        u32 size;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static const std::string magic = "ALK0"_s;

static Table read_table(io::IO &arc_io)
{
    Table table;
    auto file_count = arc_io.read_u32_le();
    size_t j = 0;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->offset = arc_io.read_u32_le();
        entry->size = arc_io.read_u32_le();
        if (entry->size)
        {
            entry->name = util::format("%03d.dat", j++);
            table.push_back(std::move(entry));
        }
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    arc_io.seek(entry.offset);
    file->io.write_from_io(arc_io, entry.size);
    file->name = entry.name;
    return file;
}

struct AlkArchive::Priv
{
    QntConverter qnt_converter;
};

AlkArchive::AlkArchive() : p(new Priv)
{
    add_transformer(&p->qnt_converter);
}

AlkArchive::~AlkArchive()
{
}

bool AlkArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void AlkArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
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
