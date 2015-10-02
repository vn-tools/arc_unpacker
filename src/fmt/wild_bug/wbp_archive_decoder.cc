#include "fmt/wild_bug/wbp_archive_decoder.h"
#include <map>
#include "util/range.h"

using namespace au;
using namespace au::fmt::wild_bug;

static const bstr magic = "ARCFORM4\x20WBUG\x20"_b;

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

static Table read_table(io::IO &arc_io)
{
    arc_io.seek(0x10);
    auto file_count = arc_io.read_u32_le();
    auto table_offset = arc_io.read_u32_le();
    auto table_size = arc_io.read_u32_le();
    arc_io.skip(8);

    std::vector<size_t> dir_offsets;
    for (auto i : util::range(0x100))
    {
        auto offset = arc_io.read_u32_le();
        if (offset)
            dir_offsets.push_back(offset);
    }

    std::vector<size_t> file_offsets;
    for (auto i : util::range(0x100))
    {
        auto offset = arc_io.read_u32_le();
        if (offset)
            file_offsets.push_back(offset);
    }

    std::map<u16, std::string> dir_names;
    for (auto &offset : dir_offsets)
    {
        arc_io.seek(offset + 1);
        auto name_size = arc_io.read_u8();
        auto dir_id = arc_io.read_u8();
        arc_io.skip(1);
        dir_names[dir_id] = arc_io.read_to_zero(name_size).str();
    }

    Table table;
    for (size_t i : util::range(file_offsets.size()))
    {
        // one file offset may contain multiple entries
        arc_io.seek(file_offsets[i]);
        do
        {
            auto old_pos = arc_io.tell();

            arc_io.skip(1);
            auto name_size = arc_io.read_u8();
            auto dir_id = arc_io.read_u8();
            arc_io.skip(1);

            TableEntry entry;
            entry.offset = arc_io.read_u32_le();
            entry.size = arc_io.read_u32_le();
            arc_io.skip(8);
            entry.name = dir_names.at(dir_id)
                + arc_io.read_to_zero(name_size).str();

            table.push_back(entry);

            arc_io.seek(old_pos + (name_size & 0xFC) + 0x18);
        }
        while (i+1 < file_offsets.size() && arc_io.tell() < file_offsets[i+1]);
    }

    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size);
    std::unique_ptr<File> file(new File);
    file->name = entry.name;
    file->io.write(data);
    return file;
}

bool WbpArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void WbpArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
        saver.save(read_file(arc_file.io, entry));
}

static auto dummy = fmt::Registry::add<WbpArchiveDecoder>("wild-bug/wbp");
