// BID archive
//
// Company:   Libido
// Engine:    -
// Extension: .BID
//
// Known games:
// - [Libido] [970613] Houkago Mania Club

#include "fmt/libido/bid_archive.h"
#include "fmt/libido/mnc_converter.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::libido;

namespace
{
    struct TableEntry
    {
        std::string name;
        size_t size;
        size_t offset;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_table(io::IO &arc_io)
{
    Table table;
    u32 data_start = arc_io.read_u32_le();
    arc_io.skip(4);
    while (arc_io.tell() < data_start)
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = arc_io.read_to_zero(16).str();
        entry->offset = arc_io.read_u32_le() + data_start;
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

struct BidArchive::Priv
{
    MncConverter mnc_converter;
};

BidArchive::BidArchive() : p(new Priv)
{
    add_transformer(&p->mnc_converter);
}

BidArchive::~BidArchive()
{
}

bool BidArchive::is_recognized_internal(File &arc_file) const
{
    auto data_start = arc_file.io.read_u32_le();
    arc_file.io.seek(data_start - 8);
    auto last_file_offset = arc_file.io.read_u32_le() + data_start;
    auto last_file_size = arc_file.io.read_u32_le();
    return last_file_offset + last_file_size == arc_file.io.size();
}

void BidArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<BidArchive>("libido/bid");
