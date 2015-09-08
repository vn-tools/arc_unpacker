// ARC archive
//
// Company:   Majiro
// Engine:    -
// Extension: .arc
//
// Known games:
// - [Empress] [150626] Closed Game

#include "fmt/majiro/arc_archive.h"
#include "fmt/majiro/rc8_converter.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::majiro;

static const bstr magic = "MajiroArcV3.000\x00"_b;

namespace
{
    struct TableEntry
    {
        std::string name;
        size_t size;
        size_t offset;
        u64 hash;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_table(io::IO &arc_io)
{
    auto file_count = arc_io.read_u32_le();
	auto names_offset = arc_io.read_u32_le();
    auto data_offset = arc_io.read_u32_le();

    Table table;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->hash = arc_io.read_u64_le();
        entry->offset = arc_io.read_u32_le();
        entry->size = arc_io.read_u32_le();
        table.push_back(std::move(entry));
    }

    arc_io.seek(names_offset);
    for (auto &entry : table)
        entry->name = util::sjis_to_utf8(arc_io.read_to_zero()).str();

    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> output_file(new File);
    arc_io.seek(entry.offset);
    output_file->io.write_from_io(arc_io, entry.size);
    output_file->name = entry.name;
    return output_file;
}

struct ArcArchive::Priv
{
    Rc8Converter rc8_converter;
};

ArcArchive::ArcArchive() : p(new Priv)
{
    add_transformer(&p->rc8_converter);
}

ArcArchive::~ArcArchive()
{
}

bool ArcArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void ArcArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<ArcArchive>("majiro/arc");
