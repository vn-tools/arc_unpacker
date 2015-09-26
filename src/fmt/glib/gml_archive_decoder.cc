#include "fmt/glib/gml_archive_decoder.h"
#include "fmt/glib/custom_lzss.h"
#include "fmt/glib/pgx_image_decoder.h"
#include "fmt/vorbis/packed_ogg_audio_decoder.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::glib;

namespace
{
    struct TableEntry final
    {
        std::string name;
        u32 offset;
        u32 size;
        bstr prefix;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static const bstr magic = "GML_ARC\x00"_b;

static std::unique_ptr<io::BufferedIO> get_header_io(
    io::IO &arc_io, size_t header_size_compressed, size_t header_size_original)
{
    bstr buffer = arc_io.read(header_size_compressed);
    for (auto i : util::range(buffer.size()))
        buffer.get<u8>()[i] ^= 0xFF;

    return std::unique_ptr<io::BufferedIO>(
        new io::BufferedIO(
            custom_lzss_decompress(buffer, header_size_original)));
}

static Table read_table(io::IO &table_io, size_t file_data_start)
{
    size_t file_count = table_io.read_u32_le();
    Table table;
    table.reserve(file_count);
    for (auto i : util::range(file_count))
    {
        std::unique_ptr<TableEntry> entry(new TableEntry);
        entry->name = table_io.read(table_io.read_u32_le()).str();
        entry->offset = table_io.read_u32_le() + file_data_start;
        entry->size = table_io.read_u32_le();
        entry->prefix = table_io.read(4);
        table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io, const TableEntry &entry, const bstr &permutation)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;

    arc_io.seek(entry.offset);
    arc_io.skip(entry.prefix.size());
    auto suffix = arc_io.read(entry.size - entry.prefix.size());
    for (auto i : util::range(suffix.size()))
        suffix.get<u8>()[i] = permutation[suffix.get<u8>()[i]];

    file->io.write(entry.prefix);
    file->io.write(suffix);
    return file;
}

struct GmlArchiveDecoder::Priv final
{
    PgxImageDecoder pgx_image_decoder;
    fmt::vorbis::PackedOggAudioDecoder packed_ogg_audio_decoder;
};

GmlArchiveDecoder::GmlArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->pgx_image_decoder);
    add_decoder(&p->packed_ogg_audio_decoder);
}

GmlArchiveDecoder::~GmlArchiveDecoder()
{
}

bool GmlArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void GmlArchiveDecoder::unpack_internal(File &arc_file, FileSaver &saver) const
{
    arc_file.io.skip(magic.size());

    u32 file_data_start = arc_file.io.read_u32_le();
    u32 header_size_original = arc_file.io.read_u32_le();
    u32 header_size_compressed = arc_file.io.read_u32_le();

    auto header_io = get_header_io(
        arc_file.io, header_size_compressed, header_size_original);
    header_io->seek(0);

    bstr permutation = header_io->read(0x100);
    Table table = read_table(*header_io, file_data_start);
    for (auto &entry : table)
        saver.save(read_file(arc_file.io, *entry, permutation));
}

static auto dummy = fmt::Registry::add<GmlArchiveDecoder>("glib/gml");
