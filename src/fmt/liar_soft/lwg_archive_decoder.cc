#include "fmt/liar_soft/lwg_archive_decoder.h"
#include "fmt/liar_soft/wcg_image_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::liar_soft;

static const bstr magic = "LG\x01\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
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

bool LwgArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    LwgArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    size_t image_width = arc_file.io.read_u32_le();
    size_t image_height = arc_file.io.read_u32_le();

    size_t file_count = arc_file.io.read_u32_le();
    arc_file.io.skip(4);
    size_t table_size = arc_file.io.read_u32_le();
    size_t file_data_start = arc_file.io.tell() + table_size + 4;

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        arc_file.io.skip(9);
        entry->offset = file_data_start + arc_file.io.read_u32_le();
        entry->size = arc_file.io.read_u32_le();
        auto name_size = arc_file.io.read_u8();
        entry->name = util::sjis_to_utf8(arc_file.io.read(name_size)).str();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> LwgArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::Registry::add<LwgArchiveDecoder>("liar/lwg");
