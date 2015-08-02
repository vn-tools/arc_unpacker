// P archive
//
// Company:   French Bread
// Engine:    -
// Extension: .p
//
// Known games:
// - Melty Blood

#include "fmt/french_bread/ex3_converter.h"
#include "fmt/french_bread/p_archive.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::french_bread;

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

static const u32 encryption_key = 0xE3DF59AC;

static std::string read_file_name(io::IO &arc_io, size_t file_id)
{
    std::string file_name = arc_io.read(60);
    for (auto i : util::range(60))
        file_name[i] ^= file_id * i * 3 + 0x3D;
    return file_name.substr(0, file_name.find('\0'));
}

static Table read_table(io::IO &arc_io)
{
    size_t file_count = arc_io.read_u32_le() ^ encryption_key;
    Table table;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = read_file_name(arc_io, i);
        entry->offset = arc_io.read_u32_le();
        entry->size = arc_io.read_u32_le() ^ encryption_key;
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io, TableEntry &entry, bool encrypted)
{
    std::unique_ptr<File> file(new File);
    std::unique_ptr<char[]> data(new char[entry.size]);
    char *ptr = data.get();
    arc_io.seek(entry.offset);
    arc_io.read(ptr, entry.size);

    for (size_t i = 0; i <= 0x2172 && i < entry.size; i++)
        data[i] ^= entry.name[i % entry.name.size()] + i + 3;

    file->name = entry.name;
    file->io.write(ptr, entry.size);

    return file;
}

struct PArchive::Priv
{
    Ex3Converter ex3_converter;
};

PArchive::PArchive() : p(new Priv)
{
    add_transformer(&p->ex3_converter);
}

PArchive::~PArchive()
{
}

bool PArchive::is_recognized_internal(File &arc_file) const
{
    u32 encrypted = arc_file.io.read_u32_le();
    size_t file_count = arc_file.io.read_u32_le() ^ encryption_key;
    if (encrypted != 0 && encrypted != 1)
        return false;
    if (file_count > arc_file.io.size() || file_count * 68 > arc_file.io.size())
        return false;
    for (auto i : util::range(file_count))
    {
        read_file_name(arc_file.io, i);
        size_t offset = arc_file.io.read_u32_le();
        size_t size = arc_file.io.read_u32_le() ^ encryption_key;
        if (offset + size > arc_file.io.size())
            return false;
    }
    return true;
}

void PArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    bool encrypted = arc_file.io.read_u32_le() == 1;
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry, encrypted));
}
