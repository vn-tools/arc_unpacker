#include "fmt/bgi/arc_archive.h"
#include "fmt/bgi/cbg_converter.h"
#include "fmt/bgi/dsc_converter.h"
#include "fmt/bgi/sound_converter.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::bgi;

static const bstr magic = "PackFile\x20\x20\x20\x20"_b;

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

static Table read_table(io::IO &arc_io)
{
    auto file_count = arc_io.read_u32_le();
    Table table;
    auto file_data_start = arc_io.tell() + file_count * 32;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = arc_io.read_to_zero(16).str();
        entry->offset = arc_io.read_u32_le() + file_data_start;
        entry->size = arc_io.read_u32_le();
        arc_io.skip(8);
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

struct ArcArchive::Priv final
{
    CbgConverter cbg_converter;
    DscConverter dsc_converter;
    SoundConverter sound_converter;
};

ArcArchive::ArcArchive() : p(new Priv)
{
    add_transformer(&p->cbg_converter);
    add_transformer(&p->dsc_converter);
    add_transformer(&p->sound_converter);
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
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<ArcArchive>("bgi/arc");
