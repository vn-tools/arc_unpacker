#include "fmt/leaf/leafpack_archive_decoder.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LEAFPACK"_b;
static const bstr key = "\x51\x42\xFE\x77\x2D\x65\x48\x7E\x0A\x8A\xE5"_b;

namespace
{
    struct TableEntry final
    {
        std::string name;
        u32 offset;
        u32 size;
    };

    using Table = std::vector<TableEntry>;
}

static void decrypt(bstr &data)
{
    for (auto i : util::range(data.size()))
        data[i] -= key[i % key.size()];
}

static Table read_table(io::IO &arc_io)
{
    arc_io.seek(magic.size());
    auto file_count = arc_io.read_u16_le();

    auto table_size = file_count * 24;
    arc_io.seek(arc_io.size() - table_size);
    auto table_data = arc_io.read(table_size);
    decrypt(table_data);

    io::BufferedIO table_io(table_data);
    Table table;
    for (auto i : util::range(file_count))
    {
        TableEntry entry;
        entry.name = table_io.read_to_zero(12).str();
        auto space_pos = entry.name.find_first_of(' ');
        if (space_pos != std::string::npos)
        {
            entry.name[space_pos] = '.';
            while (entry.name[space_pos + 1] == ' ')
                entry.name.erase(space_pos + 1, 1);
        }
        entry.offset = table_io.read_u32_le();
        entry.size = table_io.read_u32_le();
        table_io.skip(4);
        table.push_back(entry);
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size);
    decrypt(data);

    std::unique_ptr<File> file(new File);
    file->name = entry.name;
    file->io.write(data);
    return file;
}

bool LeafpackArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void LeafpackArchiveDecoder::unpack_internal(
    File &arc_file, FileSaver &saver) const
{
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
        saver.save(read_file(arc_file.io, entry));
}

static auto dummy = fmt::Registry::add<LeafpackArchiveDecoder>("leaf/leafpack");
