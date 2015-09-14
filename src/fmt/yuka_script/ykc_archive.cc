// YKC archive
//
// Company:   -
// Engine:    YukaScript
// Extension: .ykc
//
// Known games:
// - [feng] [101015] Hoshizora e Kakaru Hashi

#include "fmt/yuka_script/ykc_archive.h"
#include "fmt/yuka_script/ykg_converter.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::yuka_script;

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

static const bstr magic = "YKC001"_b;

static Table read_table(io::IO &arc_io, size_t table_offset, size_t table_size)
{
    Table table;
    size_t file_count = table_size / 20;

    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);

        arc_io.seek(table_offset + i * 20);
        size_t name_origin = arc_io.read_u32_le();
        size_t name_size = arc_io.read_u32_le();
        entry->offset = arc_io.read_u32_le();
        entry->size = arc_io.read_u32_le();
        arc_io.skip(4);

        arc_io.seek(name_origin);
        entry->name = arc_io.read(name_size).str();
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

struct YkcArchive::Priv final
{
    YkgConverter ykg_converter;
};

YkcArchive::YkcArchive() : p(new Priv)
{
    add_transformer(&p->ykg_converter);
}

YkcArchive::~YkcArchive()
{
}

bool YkcArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void YkcArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());
    arc_file.io.skip(2);
    int version = arc_file.io.read_u32_le();
    arc_file.io.skip(4);

    size_t table_offset = arc_file.io.read_u32_le();
    size_t table_size = arc_file.io.read_u32_le();
    Table table = read_table(arc_file.io, table_offset, table_size);

    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<YkcArchive>("yuka/ykc");
