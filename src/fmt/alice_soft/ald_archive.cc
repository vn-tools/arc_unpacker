// ALD archive
//
// Company:   Alice Soft
// Engine:    AliceSystem / NV-SYSTEM
// Extension: .ald
//
// Known games:
// - [Alice Soft] [971218] Atlach-Nacha
// - [Alice Soft] [971218] Ningen Gari
// - [Alice Soft] [971218] Dalk
// - [Alice Soft] [971218] Dr. Stop!
// - [Alice Soft] [971218] Rance 1 - Hikari o Motomete
// - [Alice Soft] [971218] Rance 2 - Hangyaku no Shoujo-tachi
// - [Alice Soft] [971218] Rance 3 - Leazas Kanraku
// - [Alice Soft] [971218] Toushin Toshi
// - [Alice Soft] [971218] Zero
// - [Alice Soft] [011130] Daiakuji

#include "fmt/alice_soft/ald_archive.h"
#include "fmt/alice_soft/pms_converter.h"
#include "fmt/alice_soft/vsp_converter.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

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

static u32 read_24_le(io::IO &io)
{
    return (io.read_u8() << 8) | (io.read_u8() << 16) | (io.read_u8() << 24);
}

static Table read_table(io::IO &arc_io)
{
    auto file_count = read_24_le(arc_io) / 3;

    std::vector<size_t> offsets(file_count);
    for (auto i : util::range(file_count))
        offsets[i] = read_24_le(arc_io);

    Table table;
    for (auto &offset : offsets)
    {
        if (!offset)
            break;
        arc_io.seek(offset);
        auto header_size = arc_io.read_u32_le();
        if (arc_io.tell() + header_size < arc_io.size())
        {
            std::unique_ptr<TableEntry> entry(new TableEntry);
            entry->size = arc_io.read_u32_le();
            arc_io.skip(8);
            auto name = arc_io.read_to_zero(header_size - 16);
            entry->name = util::sjis_to_utf8(name).str();
            entry->offset = arc_io.tell();
            table.push_back(std::move(entry));
        }
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

struct AldArchive::Priv final
{
    PmsConverter pms_converter;
    VspConverter vsp_converter;
};

AldArchive::AldArchive() : p(new Priv)
{
    add_transformer(&p->pms_converter);
    add_transformer(&p->vsp_converter);
}

AldArchive::~AldArchive()
{
}

bool AldArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("ald");
}

void AldArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        file_saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<AldArchive>("alice/ald");
