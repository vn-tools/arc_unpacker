// SAR archive
//
// Company:   -
// Engine:    NScripter
// Extension: .sar
//
// Known games:
// - Tsukihime

#include "fmt/nscripter/sar_archive.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nscripter;

namespace
{
    struct TableEntry
    {
        std::string name;
        u32 offset;
        u32 size;
    };
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;

    arc_io.seek(entry.offset);
    file->io.write_from_io(arc_io, entry.size);

    return file;
}

bool SarArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("sar");
}

void SarArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    u16 file_count = arc_file.io.read_u16_be();
    u32 offset_to_files = arc_file.io.read_u32_be();

    std::vector<std::unique_ptr<TableEntry>> table;
    table.reserve(file_count);
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = arc_file.io.read_to_zero().str();
        entry->offset = arc_file.io.read_u32_be() + offset_to_files;
        entry->size = arc_file.io.read_u32_be();
        table.push_back(std::move(entry));
    }

    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<SarArchive>("nscripter/sar");
