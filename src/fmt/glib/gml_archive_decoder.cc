#include "fmt/glib/gml_archive_decoder.h"
#include "fmt/glib/custom_lzss.h"
#include "fmt/glib/pgx_image_decoder.h"
#include "fmt/vorbis/packed_ogg_audio_decoder.h"
#include "io/buffered_io.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::glib;

static const bstr magic = "GML_ARC\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
        bstr prefix;
    };

    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        bstr permutation;
    };
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

std::unique_ptr<fmt::ArchiveMeta>
    GmlArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    auto file_data_start = arc_file.io.read_u32_le();
    auto table_size_orig = arc_file.io.read_u32_le();
    auto table_size_comp = arc_file.io.read_u32_le();
    auto table_data = arc_file.io.read(table_size_comp);
    for (auto i : util::range(table_data.size()))
        table_data[i] ^= 0xFF;
    table_data = custom_lzss_decompress(table_data, table_size_orig);
    io::BufferedIO table_io(table_data);

    auto meta = std::make_unique<ArchiveMetaImpl>();
    meta->permutation = table_io.read(0x100);

    auto file_count = table_io.read_u32_le();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = table_io.read(table_io.read_u32_le()).str();
        entry->offset = table_io.read_u32_le() + file_data_start;
        entry->size = table_io.read_u32_le();
        entry->prefix = table_io.read(4);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> GmlArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);

    arc_file.io.seek(entry->offset);
    arc_file.io.skip(entry->prefix.size());
    auto suffix = arc_file.io.read(entry->size - entry->prefix.size());
    for (auto i : util::range(suffix.size()))
        suffix[i] = meta->permutation[suffix.get<u8>()[i]];

    auto output_file = std::make_unique<File>();
    output_file->name = entry->name;
    output_file->io.write(entry->prefix);
    output_file->io.write(suffix);
    return output_file;
}

static auto dummy = fmt::Registry::add<GmlArchiveDecoder>("glib/gml");
