#include "fmt/tactics/arc_archive.h"
#include "err.h"
#include "fmt/microsoft/dds_converter.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::tactics;

static const bstr magic = "TACTICS_ARC_FILE"_b;

namespace
{
    struct TableEntry final
    {
        std::string name;
        size_t size_comp;
        size_t size_orig;
        size_t offset;
        bstr key;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_table_v0(io::IO &arc_io)
{
    auto size_comp = arc_io.read_u32_le();
    auto size_orig = arc_io.read_u32_le();
    auto file_count = arc_io.read_u32_le();
    if (size_comp > 1024 * 1024 * 10)
        throw err::BadDataSizeError();

    arc_io.skip(4);
    auto table_buf = arc_io.read(size_comp);
    auto data_start = arc_io.tell();

    for (auto &c : table_buf)
        c ^= 0xFF;
    io::BufferedIO table_io(
        util::pack::lzss_decompress_bytewise(table_buf, size_orig));

    auto key = table_io.read_to_zero();

    Table table(file_count);
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->offset = table_io.read_u32_le() + data_start;
        entry->size_comp = table_io.read_u32_le();
        entry->size_orig = table_io.read_u32_le();
        entry->key = key;
        auto name_size = table_io.read_u32_le();

        table_io.skip(8);
        entry->name = util::sjis_to_utf8(table_io.read(name_size)).str();
        table[i] = std::move(entry);
    }
    return table;
}

static Table read_table_v1(io::IO &arc_io)
{
    static const bstr key = "mlnebzqm"_b; //found in .exe
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
        entry->key = key;
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
    if (entry.key.size())
        for (auto i : util::range(data.size()))
            data[i] ^= entry.key[i % entry.key.size()];
    if (entry.size_orig)
        data = util::pack::lzss_decompress_bytewise(data, entry.size_orig);
    output_file->name = entry.name;
    output_file->io.write(data);
    return output_file;
}

struct ArcArchive::Priv final
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
    std::unique_ptr<Table> table;
    std::vector<std::function<Table(io::IO &)>> table_readers
    {
        read_table_v0,
        read_table_v1
    };

    for (auto table_reader : table_readers)
    {
        arc_file.io.seek(magic.size());
        try
        {
            table = std::make_unique<Table>(table_reader(arc_file.io));
        }
        catch (std::exception &e)
        {
            continue;
        }
    }

    if (!table)
        throw err::NotSupportedError("Archive is encrypted in unknown way.");

    for (auto &entry : *table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<ArcArchive>("tactics/arc");
