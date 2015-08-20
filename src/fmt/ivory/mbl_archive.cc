// MBL archive
//
// Company:   Ivory
// Engine:    MarbleEngine
// Extension: .mbl
//
// Known games:
// - Wanko to Kurasou

#include "fmt/ivory/mbl_archive.h"
#include "fmt/ivory/prs_converter.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::ivory;

namespace
{
    enum Version
    {
        Unknown,
        Version1,
        Version2,
    };

    struct TableEntry
    {
        std::string name;
        size_t offset;
        size_t size;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static int check_version(io::IO &arc_io, size_t file_count, size_t name_size)
{
    arc_io.skip((file_count - 1) * (name_size + 8));
    arc_io.skip(name_size);
    auto last_file_offset = arc_io.read_u32_le();
    auto last_file_size = arc_io.read_u32_le();
    return last_file_offset + last_file_size == arc_io.size();
}

static Version get_version(io::IO &arc_io)
{
    auto file_count = arc_io.read_u32_le();
    if (check_version(arc_io, file_count, 16))
        return Version::Version1;

    arc_io.seek(4);
    auto name_size = arc_io.read_u32_le();
    if (check_version(arc_io, file_count, name_size))
        return Version::Version2;

    return Version::Unknown;
}

static Table read_table(io::IO &arc_io, Version version)
{
    Table table;
    auto file_count = arc_io.read_u32_le();
    auto name_size = version == Version::Version2 ? arc_io.read_u32_le() : 16;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = util::sjis_to_utf8(arc_io.read_to_zero(name_size)).str();
        entry->offset = arc_io.read_u32_le();
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

struct MblArchive::Priv
{
    PrsConverter prs_converter;
};

MblArchive::MblArchive() : p(new Priv)
{
    add_transformer(&p->prs_converter);
}

MblArchive::~MblArchive()
{
}

bool MblArchive::is_recognized_internal(File &arc_file) const
{
    return get_version(arc_file.io) != Version::Unknown;
}

void MblArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto version = get_version(arc_file.io);
    arc_file.io.seek(0);

    auto table = read_table(arc_file.io, version);

    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}
