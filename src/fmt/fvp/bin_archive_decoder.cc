#include "fmt/fvp/bin_archive_decoder.h"
#include "fmt/fvp/nvsg_image_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fvp;

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
    size_t file_count = arc_io.read_u32_le();
    arc_io.skip(4);

    size_t names_start = file_count * 12 + 8;

    Table table;
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        size_t name_offset = arc_io.read_u32_le();
        arc_io.peek(names_start + name_offset, [&]()
        {
            entry->name = util::sjis_to_utf8(arc_io.read_to_zero()).str();
        });
        entry->offset = arc_io.read_u32_le();
        entry->size = arc_io.read_u32_le();
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(io::IO &arc_io, TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    arc_io.seek(entry.offset);
    file->io.write_from_io(arc_io, entry.size);
    file->name = entry.name;
    return file;
}

struct BinArchiveDecoder::Priv final
{
    NvsgImageDecoder nvsg_image_decoder;
};

BinArchiveDecoder::BinArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->nvsg_image_decoder);
}

BinArchiveDecoder::~BinArchiveDecoder()
{
}

bool BinArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.has_extension("bin");
}

void BinArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    Table table = read_table(arc_file.io);
    for (auto &entry : table)
    {
        auto file = read_file(arc_file.io, *entry);
        file->guess_extension();
        saver.save(std::move(file));
    }
}

static auto dummy = fmt::Registry::add<BinArchiveDecoder>("fvp/bin");
