#include "fmt/libido/bid_archive_decoder.h"
#include "fmt/libido/mnc_image_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::libido;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

struct BidArchiveDecoder::Priv final
{
    MncImageDecoder mnc_image_decoder;
};

BidArchiveDecoder::BidArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->mnc_image_decoder);
}

BidArchiveDecoder::~BidArchiveDecoder()
{
}

bool BidArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    auto data_start = arc_file.io.read_u32_le();
    arc_file.io.seek(data_start - 8);
    auto last_file_offset = arc_file.io.read_u32_le() + data_start;
    auto last_file_size = arc_file.io.read_u32_le();
    return last_file_offset + last_file_size == arc_file.io.size();
}

std::unique_ptr<fmt::ArchiveMeta>
    BidArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto meta = std::make_unique<ArchiveMeta>();
    u32 data_start = arc_file.io.read_u32_le();
    arc_file.io.skip(4);
    while (arc_file.io.tell() < data_start)
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.io.read_to_zero(16).str();
        entry->offset = arc_file.io.read_u32_le() + data_start;
        entry->size = arc_file.io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> BidArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::Registry::add<BidArchiveDecoder>("libido/bid");
