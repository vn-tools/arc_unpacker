#include "fmt/propeller/mpk_archive_decoder.h"
#include "fmt/propeller/mgr_archive_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::propeller;

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
    Table table;
    size_t table_offset = arc_io.read_u32_le();
    size_t file_count = arc_io.read_u32_le();
    table.reserve(file_count);

    arc_io.seek(table_offset);
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);

        auto name = arc_io.read(32);
        u8 key8 = name[31];
        u32 key32 = (key8 << 24) | (key8 << 16) | (key8 << 8) | key8;

        for (auto i : util::range(32))
            name[i] ^= key8;

        entry->name = util::sjis_to_utf8(name).str();
        if (entry->name[0] == '\\')
            entry->name = entry->name.substr(1);
        entry->name.erase(entry->name.find('\x00'));

        entry->offset = arc_io.read_u32_le() ^ key32;
        entry->size = arc_io.read_u32_le() ^ key32;

        table.push_back(std::move(entry));
    }

    if (file_count == 0)
        return table;

    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;
    arc_io.seek(entry.offset);
    file->io.write_from_io(arc_io, entry.size);
    return file;
}

struct MpkArchiveDecoder::Priv final
{
    MgrArchiveDecoder mgr_archive_decoder;
};

MpkArchiveDecoder::MpkArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->mgr_archive_decoder);
}

MpkArchiveDecoder::~MpkArchiveDecoder()
{
}

bool MpkArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("mpk");
}

void MpkArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    auto table = read_table(arc_file.io);
    for (auto &entry : table)
        saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<MpkArchiveDecoder>("propeller/mpk");
