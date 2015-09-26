#include "fmt/real_live/ovk_archive_decoder.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::real_live;

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

bool OvkArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("ovk");
}

void OvkArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<OvkArchiveDecoder>("rl/ovk");
