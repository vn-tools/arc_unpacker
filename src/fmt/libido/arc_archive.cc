// ARC archive
//
// Company:   Libido
// Engine:    -
// Extension: .arc
//
// Known games:
// - Cherry Boy, Innocent Girl
// - Fifteen ~School Girls Digital Tokuhon~
// - Girl Friends

#include <algorithm>
#include "fmt/libido/arc_archive.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::libido;

namespace
{
    struct TableEntry
    {
        std::string name;
        size_t size_compressed;
        size_t size_original;
        size_t offset;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_table(io::IO &arc_io)
{
    Table table;
    u32 file_count = arc_io.read_u32_le();
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        auto tmp = arc_io.read(20);
        for (auto i : util::range(tmp.size()))
            tmp[i] ^= tmp[tmp.size() - 1];
        entry->name = tmp.str(true);
        entry->size_original = arc_io.read_u32_le();
        entry->size_compressed = arc_io.read_u32_le();
        entry->offset = arc_io.read_u32_le();
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);

    arc_io.seek(entry.offset);
    auto compressed_data = arc_io.read(entry.size_compressed);
    auto decompressed_data = util::pack::lzss_decompress_bytewise(
        compressed_data, entry.size_original);
    file->io.write(decompressed_data);
    file->name = entry.name;
    return file;
}

bool ArcArchive::is_recognized_internal(File &arc_file) const
{
    auto file_count = arc_file.io.read_u32_le();
    if (file_count)
    {
        arc_file.io.skip((file_count - 1) * 32 + 24);
        arc_file.io.seek(arc_file.io.read_u32_le() + arc_file.io.read_u32_le());
    }
    else
        arc_file.io.skip(1);
    return arc_file.io.eof();
}

void ArcArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<ArcArchive>("libido/arc");
