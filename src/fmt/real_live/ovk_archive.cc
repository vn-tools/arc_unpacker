// OVK voice archive
//
// Company:   -
// Engine:    RealLive
// Extension: .ovk
//
// Known games:
// - [Hamham Soft] [071221] Imouto ni! Sukumizu Kisetara Nugasanai!
// - [Key] [070928] Little Busters!
// - [Key] [080229] Clannad

#include "fmt/real_live/ovk_archive.h"
#include "util/range.h"
#include "util/format.h"

using namespace au;
using namespace au::fmt::real_live;

namespace
{
    struct TableEntry
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
        entry->size = arc_io.read_u32_le();
        entry->offset = arc_io.read_u32_le();
        entry->name = util::format("sample%05d", arc_io.read_u32_le());
        arc_io.skip(4);
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

bool OvkArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("ovk");
}

void OvkArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<OvkArchive>("rl/ovk");
