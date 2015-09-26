#include "fmt/cherry_soft/myk_archive_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::cherry_soft;

static const bstr magic = "MYK00\x1A\x00\x00"_b;

namespace
{
    struct TableEntry final
    {
        std::string name;
        size_t size;
        size_t offset;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_table(io::IO &arc_io)
{
    auto file_count = arc_io.read_u16_le();
    auto table_offset = arc_io.read_u32_le();
    arc_io.seek(table_offset);

    Table table(file_count);
    auto current_offset = 16;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = arc_io.read_to_zero(12).str();
        entry->offset = current_offset;
        entry->size = arc_io.read_u32_le();
        current_offset += entry->size;
        table[i] = std::move(entry);
    }

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

bool MykArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void MykArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    arc_file.io.skip(magic.size());
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
        saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<MykArchiveDecoder>("cherry/myk");
