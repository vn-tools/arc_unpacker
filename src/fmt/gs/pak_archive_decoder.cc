#include "fmt/gs/pak_archive_decoder.h"
#include "fmt/gs/gs_image_decoder.h"
#include "io/buffered_io.h"
#include "util/pack/lzss.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::gs;

static const bstr magic = "DataPack5\x00\x00\x00\x00\x00\x00\x00"_b;

namespace
{
    struct TableEntry final
    {
        std::string name;
        u32 offset;
        u32 size;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static Table read_table(io::IO &arc_io, size_t version)
{
    auto entry_size = (version >> 16) < 5 ? 0x48 : 0x68;

    auto table_size_comp = arc_io.read_u32_le();
    auto key = arc_io.read_u32_le();
    auto file_count = arc_io.read_u32_le();
    auto data_offset = arc_io.read_u32_le();
    auto table_offset = arc_io.read_u32_le();

    auto table_size_orig = file_count * entry_size;

    arc_io.seek(table_offset);
    auto table_data = arc_io.read(table_size_comp);
    for (auto i : util::range(table_data.size()))
        table_data[i] ^= i & key;
    table_data = util::pack::lzss_decompress_bytewise(
        table_data, table_size_orig);
    io::BufferedIO table_io(table_data);

    Table table;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        table_io.seek(entry_size * i);
        entry->name = table_io.read_to_zero(0x40).str();
        entry->offset = table_io.read_u32_le() + data_offset;
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

struct PakArchiveDecoder::Priv final
{
    GsImageDecoder gs_image_decoder;
};

PakArchiveDecoder::PakArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->gs_image_decoder);
}

PakArchiveDecoder::~PakArchiveDecoder()
{
}

bool PakArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void PakArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    arc_file.io.skip(0x30);
    auto version = arc_file.io.read_u32_le();
    auto table = read_table(arc_file.io, version);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<PakArchiveDecoder>("gs/pak");
