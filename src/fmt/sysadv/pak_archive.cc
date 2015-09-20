#include "fmt/sysadv/pak_archive.h"
#include "fmt/sysadv/pga_converter.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::sysadv;

static const bstr magic = "\x05PACK2"_b;

namespace
{
    struct TableEntry final
    {
        std::string name;
        size_t offset;
        size_t size;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_table(io::IO &arc_io)
{
    auto file_count = arc_io.read_u32_le();
    Table table;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        auto name = arc_io.read(arc_io.read_u8());
        for (auto i : util::range(name.size()))
            name[i] ^= 0xFF;
        entry->name = name.str();
        entry->offset = arc_io.read_u32_le();
        entry->size = arc_io.read_u32_le();
        table.push_back(std::move(entry));
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

struct PakArchive::Priv final
{
    PgaConverter pga_converter;
};

PakArchive::PakArchive() : p(new Priv)
{
    add_transformer(&p->pga_converter);
}

PakArchive::~PakArchive()
{
}

bool PakArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void PakArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<PakArchive>("sysadv/pak");
