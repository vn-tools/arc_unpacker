// ARC archive
//
// Company:   Tactics
// Engine:    -
// Extension: .arc
//
// Known games:
// - Dainikai Imouto Senbatsu☆Sousenkyo 2

#include "fmt/microsoft/dds_converter.h"
#include "fmt/tactics/arc_archive.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::tactics;

static const bstr magic = "TACTICS_ARC_FILE"_b;
static const bstr key = "mlnebzqm"_b;

namespace
{
    struct TableEntry
    {
        std::string name;
        size_t size_comp;
        size_t size_orig;
        size_t offset;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_table(io::IO &arc_io)
{
    Table table;
    while (!arc_io.eof())
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->size_comp = arc_io.read_u32_le();
        if (!entry->size_comp)
            break;
        entry->size_orig = arc_io.read_u32_le();
        auto name_size = arc_io.read_u32_le();
        arc_io.skip(8);
        entry->name = util::sjis_to_utf8(arc_io.read(name_size)).str();
        entry->offset = arc_io.tell();
        arc_io.skip(entry->size_comp);
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> output_file(new File);
    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size_comp);
    for (auto i : util::range(data.size()))
        data[i] ^= key[i % key.size()];
    if (entry.size_orig)
        data = util::pack::lzss_decompress_bytewise(data, entry.size_orig);
    output_file->name = entry.name;
    output_file->io.write(data);
    return output_file;
}

struct ArcArchive::Priv
{
    fmt::microsoft::DdsConverter dds_converter;
};

ArcArchive::ArcArchive() : p(new Priv)
{
    add_transformer(&p->dds_converter);
}

ArcArchive::~ArcArchive()
{
}

bool ArcArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void ArcArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<ArcArchive>("tactics/arc");
