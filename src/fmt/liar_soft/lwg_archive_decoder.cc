#include "fmt/liar_soft/lwg_archive_decoder.h"
#include "fmt/liar_soft/wcg_image_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::liar_soft;

namespace
{
    struct TableEntry final
    {
        std::string name;
        size_t offset;
        size_t size;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static const bstr magic = "LG\x01\x00"_b;

static Table read_table(io::IO &arc_io)
{
    size_t file_count = arc_io.read_u32_le();
    arc_io.skip(4);
    size_t table_size = arc_io.read_u32_le();
    size_t file_start = arc_io.tell() + table_size + 4;

    Table table;
    table.reserve(file_count);
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        arc_io.skip(9);
        entry->offset = file_start + arc_io.read_u32_le();
        entry->size = arc_io.read_u32_le();
        entry->name = util::sjis_to_utf8(arc_io.read(arc_io.read_u8())).str();
        table.push_back(std::move(entry));
    }
    size_t file_data_size = arc_io.read_u32_le();
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

struct LwgArchiveDecoder::Priv final
{
    WcgImageDecoder wcg_image_decoder;
};

LwgArchiveDecoder::LwgArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->wcg_image_decoder);
    add_decoder(this);
}

LwgArchiveDecoder::~LwgArchiveDecoder()
{
}

bool LwgArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void LwgArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    arc_file.io.skip(magic.size());
    size_t image_width = arc_file.io.read_u32_le();
    size_t image_height = arc_file.io.read_u32_le();

    Table table = read_table(arc_file.io);
    for (auto &entry : table)
        saver.save(read_file(arc_file.io, *entry));
}

static auto dummy = fmt::Registry::add<LwgArchiveDecoder>("liar/lwg");
