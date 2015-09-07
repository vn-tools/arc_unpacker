// AFA archive
//
// Company:   Alice Soft
// Engine:    -
// Extension: .afa
//
// Known games:
// - [Alice Soft] [011130] Daiakuji

#include "fmt/alice_soft/afa_archive.h"
#include "fmt/alice_soft/aff_converter.h"
#include "fmt/alice_soft/ajp_converter.h"
#include "fmt/alice_soft/qnt_converter.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const bstr magic1 = "AFAH"_b;
static const bstr magic2 = "AlicArch"_b;
static const bstr magic3 = "INFO"_b;

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

static Table read_table(io::IO &arc_io)
{
    auto file_data_start = arc_io.read_u32_le();
    if (arc_io.read(magic3.size()) != magic3)
        throw std::runtime_error("Corrupt header");

    Table table;
    auto table_size_compressed = arc_io.read_u32_le();
    auto table_size_original = arc_io.read_u32_le();
    auto file_count = arc_io.read_u32_le();

    io::BufferedIO table_io(
        util::pack::zlib_inflate(
            arc_io.read(table_size_compressed)));

    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);

        table_io.skip(4);
        auto name_size = table_io.read_u32_le();
        entry->name = util::sjis_to_utf8(
            table_io.read_to_zero(name_size)).str();

        table_io.skip(4 * 3); //for some games, apparently this is 4 * 2
        entry->offset = table_io.read_u32_le() + file_data_start;
        entry->size = table_io.read_u32_le();
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

struct AfaArchive::Priv
{
    AffConverter aff_converter;
    AjpConverter ajp_converter;
    QntConverter qnt_converter;
};

AfaArchive::AfaArchive() : p(new Priv)
{
    add_transformer(&p->aff_converter);
    add_transformer(&p->ajp_converter);
    add_transformer(&p->qnt_converter);
}

AfaArchive::~AfaArchive()
{
}

bool AfaArchive::is_recognized_internal(File &arc_file) const
{
    if (arc_file.io.read(magic1.size()) != magic1)
        return false;
    arc_file.io.skip(4);
    return arc_file.io.read(magic2.size()) == magic2;
}

void AfaArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic1.size());
    arc_file.io.skip(4);
    arc_file.io.skip(magic2.size());
    arc_file.io.skip(4 * 2);

    Table table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<AfaArchive>("alice/afa");
