#include "fmt/alice_soft/alk_archive_decoder.h"
#include "fmt/alice_soft/qnt_image_decoder.h"
#include "util/format.h"
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

static const bstr magic = "ALK0"_b;

static Table read_table(io::IO &arc_io)
{
    Table table;
    auto file_count = arc_io.read_u32_le();
    size_t j = 0;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->offset = arc_io.read_u32_le();
        entry->size = arc_io.read_u32_le();
        if (entry->size)
        {
            entry->name = util::format("%03d.dat", j++);
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

struct AlkArchiveDecoder::Priv final
{
    QntImageDecoder qnt_image_decoder;
};

AlkArchiveDecoder::AlkArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->qnt_image_decoder);
}

AlkArchiveDecoder::~AlkArchiveDecoder()
{
}

bool AlkArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void AlkArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    arc_file.io.skip(magic.size());
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<AlkArchiveDecoder>("alice/alk");
