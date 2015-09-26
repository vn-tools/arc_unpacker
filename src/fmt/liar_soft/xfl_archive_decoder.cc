#include "fmt/liar_soft/xfl_archive_decoder.h"
#include "fmt/liar_soft/lwg_archive_decoder.h"
#include "fmt/liar_soft/wcg_image_decoder.h"
#include "fmt/vorbis/packed_ogg_audio_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::liar_soft;

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

static const bstr magic = "LB\x01\x00"_b;

static Table read_table(io::IO &arc_io)
{
    Table table;
    size_t table_size = arc_io.read_u32_le();
    size_t file_count = arc_io.read_u32_le();
    size_t file_start = arc_io.tell() + table_size;
    table.reserve(file_count);
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = util::sjis_to_utf8(arc_io.read_to_zero(0x20)).str();
        entry->offset = file_start + arc_io.read_u32_le();
        entry->size = arc_io.read_u32_le();
        table.push_back(std::move(entry));
    }
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

struct XflArchiveDecoder::Priv final
{
    LwgArchiveDecoder lwg_archive_decoder;
    WcgImageDecoder wcg_image_decoder;
    fmt::vorbis::PackedOggAudioDecoder packed_ogg_audio_decoder;
};

XflArchiveDecoder::XflArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->wcg_image_decoder);
    add_decoder(&p->lwg_archive_decoder);
    add_decoder(&p->packed_ogg_audio_decoder);
    add_decoder(this);
}

XflArchiveDecoder::~XflArchiveDecoder()
{
}

bool XflArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void XflArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
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

static auto dummy = fmt::Registry::add<XflArchiveDecoder>("liar/xfl");
