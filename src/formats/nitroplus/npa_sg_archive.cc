// NPA archive
//
// Company:   Nitroplus
// Engine:    -
// Extension: .npa
//
// Known games:
// - Steins;Gate

#include "formats/nitroplus/npa_sg_archive.h"
#include "io/buffered_io.h"
#include "util/encoding.h"

using namespace au;
using namespace au::fmt::nitroplus;

namespace
{
    typedef struct
    {
        std::string name;
        size_t offset;
        size_t size;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;
}

static const std::string key("\xBD\xAA\xBC\xB4\xAB\xB6\xBC\xB4", 8);

static void decrypt(char *data, size_t data_size)
{
    for (size_t i = 0; i < data_size; i++)
        data[i] ^= key[i % key.length()];
}

static Table read_table(io::IO &table_io, const io::IO &arc_io)
{
    Table table;
    size_t file_count = table_io.read_u32_le();
    for (size_t i = 0; i < file_count; i++)
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = util::convert_encoding(
            table_io.read(table_io.read_u32_le()), "utf-16le", "utf-8");
        entry->size = table_io.read_u32_le();
        entry->offset = table_io.read_u32_le();
        table_io.skip(4);
        if (entry->offset + entry->size > arc_io.size())
            throw std::runtime_error("Bad offset to file");
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    std::unique_ptr<char[]> data(new char[entry.size]);
    arc_io.seek(entry.offset);
    arc_io.read(data.get(), entry.size);
    decrypt(data.get(), entry.size);
    file->name = entry.name;
    file->io.write(data.get(), entry.size);
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
        throw std::runtime_error("Bad table size");

    std::unique_ptr<char[]> table_bytes(new char[table_size]);
    arc_file.io.read(table_bytes.get(), table_size);
    decrypt(table_bytes.get(), table_size);

    io::BufferedIO table_io(table_bytes.get(), table_size);
    Table table = read_table(table_io, arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}
