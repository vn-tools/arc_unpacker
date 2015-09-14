// PAK2 archive
//
// Company:   Nitroplus
// Engine:    -
// Extension: .pak
//
// Known games:
// - [Nitroplus] [031226] Saya no Uta

#include "fmt/nitroplus/pak_archive.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nitroplus;

static const bstr magic = "\x02\x00\x00\x00"_b;

namespace
{
    struct TableEntry final
    {
        std::string name;
        size_t offset;
        size_t flags;
        size_t size_original;
        size_t size_compressed;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_table(io::IO &arc_io)
{
    auto file_count = arc_io.read_u32_le();
    auto table_size_original = arc_io.read_u32_le();
    auto table_size_compressed = arc_io.read_u32_le();
    arc_io.skip(0x104);

    io::BufferedIO table_io(
        util::pack::zlib_inflate(
            arc_io.read(table_size_compressed)));

    Table table;
    auto file_data_offset = arc_io.tell();
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        size_t file_name_size = table_io.read_u32_le();
        entry->name = util::sjis_to_utf8(table_io.read(file_name_size)).str();
        entry->offset = table_io.read_u32_le() + file_data_offset;
        entry->size_original = table_io.read_u32_le();
        table_io.skip(4);
        entry->flags = table_io.read_u32_le();
        entry->size_compressed = table_io.read_u32_le();
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    arc_io.seek(entry.offset);
    auto data = entry.flags > 0
        ? util::pack::zlib_inflate(arc_io.read(entry.size_compressed))
        : arc_io.read(entry.size_original);
    std::unique_ptr<File> file(new File);
    file->io.write(data);
    file->name = entry.name;
    return file;
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
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<PakArchive>("nitro/pak");
