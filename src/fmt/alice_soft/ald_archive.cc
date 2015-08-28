// ALD archive
//
// Company:   Alice Soft
// Engine:    -
// Extension: .ald
//
// Known games:
// - Daiakuji

#include "fmt/alice_soft/ald_archive.h"
#include "util/encoding.h"
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

static u32 read_24_le(io::IO &io)
{
    return (io.read_u8() << 8) | (io.read_u8() << 16) | (io.read_u8() << 24);
}

static Table read_table(io::IO &arc_io)
{
    Table table;
    auto file_count = read_24_le(arc_io) / 3;

    for (auto i : util::range(file_count))
    {
        auto offset = read_24_le(arc_io);
        if (!offset)
            break;

        arc_io.peek(offset, [&]()
        {
            auto header_size = arc_io.read_u32_le();
            if (arc_io.tell() + header_size < arc_io.size())
            {
                std::unique_ptr<TableEntry> entry(new TableEntry);
                entry->size = arc_io.read_u32_le();
                arc_io.skip(8);
                entry->name = arc_io.read_to_zero(header_size - 16).str();
                entry->offset = arc_io.tell();
                table.push_back(std::move(entry));
            }
        });
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

bool AldArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("ald");
}

void AldArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<AldArchive>("alice/ald");
