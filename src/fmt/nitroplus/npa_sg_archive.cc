// NPA archive
//
// Company:   Nitroplus
// Engine:    -
// Extension: .npa
//
// Known games:
// - [Nitroplus] [100826] Steins;Gate

#include "err.h"
#include "fmt/nitroplus/npa_sg_archive.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nitroplus;

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

static const bstr key = "\xBD\xAA\xBC\xB4\xAB\xB6\xBC\xB4"_b;

static void decrypt(bstr &data)
{
    for (auto i : util::range(data.size()))
        data[i] ^= key[i % key.size()];
}

static Table read_table(io::IO &table_io, const io::IO &arc_io)
{
    Table table;
    size_t file_count = table_io.read_u32_le();
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = util::convert_encoding(
            table_io.read(table_io.read_u32_le()), "utf-16le", "utf-8").str();
        entry->size = table_io.read_u32_le();
        entry->offset = table_io.read_u32_le();
        table_io.skip(4);
        if (entry->offset + entry->size > arc_io.size())
            throw err::BadDataOffsetError();
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    auto data = arc_io.read(entry.size);
    arc_io.seek(entry.offset);
    decrypt(data);
    file->name = entry.name;
    file->io.write(data);
    return file;
}

bool NpaSgArchive::is_recognized_internal(File &arc_file) const
{
    if (!arc_file.has_extension("npa"))
        return false;
    size_t table_size = arc_file.io.read_u32_le();
    return table_size < arc_file.io.size();
}

void NpaSgArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    size_t table_size = arc_file.io.read_u32_le();
    if (table_size > arc_file.io.size())
        throw err::BadDataSizeError();

    auto table_bytes = arc_file.io.read(table_size);
    decrypt(table_bytes);

    io::BufferedIO table_io(table_bytes);
    Table table = read_table(table_io, arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<NpaSgArchive>("nitro/npa-sg");
