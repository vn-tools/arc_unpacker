// PAC archive
//
// Company:   Riddle Soft
// Engine:    Rage's Adventure Game Engine
// Extension: .pac
//
// Known games:
// - Brightia
// - Caliz

#include "fmt/riddle_soft/cmp_converter.h"
#include "fmt/riddle_soft/pac_archive.h"
#include "util/require.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::riddle_soft;

static const bstr magic = "PAC1"_b;

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

static Table read_table(io::IO &arc_io)
{
    auto file_count = arc_io.read_u32_le();
    Table table;
    auto file_data_start = arc_io.tell() + file_count * 32;
    auto current_file_offset = 0;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = arc_io.read_to_zero(16).str();
        entry->size = arc_io.read_u32_le();
        auto prefix = arc_io.read(4);
        auto unk1 = arc_io.read_u32_le();
        auto unk2 = arc_io.read_u32_le();
        util::require(unk1 == unk2);
        entry->offset = file_data_start + current_file_offset;
        current_file_offset += entry->size;
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

struct PacArchive::Priv
{
    CmpConverter cmp_converter;
};

PacArchive::PacArchive() : p(new Priv)
{
    add_transformer(&p->cmp_converter);
}

PacArchive::~PacArchive()
{
}

bool PacArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void PacArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<PacArchive>("riddle/pac");
