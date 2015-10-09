#include "fmt/liar_soft/xfl_archive_decoder.h"
#include "fmt/liar_soft/lwg_archive_decoder.h"
#include "fmt/liar_soft/wcg_image_decoder.h"
#include "fmt/vorbis/packed_ogg_audio_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::liar_soft;

static const bstr magic = "LB\x01\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
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

std::unique_ptr<fmt::ArchiveMeta>
    XflArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    auto table_size = arc_file.io.read_u32_le();
    auto file_count = arc_file.io.read_u32_le();
    auto file_start = arc_file.io.tell() + table_size;
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = util::sjis_to_utf8(arc_file.io.read_to_zero(0x20)).str();
        entry->offset = file_start + arc_file.io.read_u32_le();
        entry->size = arc_file.io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> XflArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

static auto dummy = fmt::Registry::add<XflArchiveDecoder>("liar/xfl");
