#include "fmt/nsystem/fjsys_archive_decoder.h"
#include "fmt/nsystem/mgd_image_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nsystem;

static const bstr magic = "FJSYS\x00\x00\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

struct FjsysArchiveDecoder::Priv final
{
    MgdImageDecoder mgd_image_decoder;
};

FjsysArchiveDecoder::FjsysArchiveDecoder() : p(new Priv)
{
    add_decoder(&p->mgd_image_decoder);
}

FjsysArchiveDecoder::~FjsysArchiveDecoder()
{
}

bool FjsysArchiveDecoder::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    FjsysArchiveDecoder::read_meta(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    auto header_size = arc_file.io.read_u32_le();
    auto file_names_size = arc_file.io.read_u32_le();
    auto file_names_start = header_size - file_names_size;
    auto file_count = arc_file.io.read_u32_le();
    arc_file.io.skip(64);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        size_t file_name_offset = arc_file.io.read_u32_le();
        entry->size = arc_file.io.read_u32_le();
        entry->offset = arc_file.io.read_u64_le();
        arc_file.io.peek(file_name_offset + file_names_start, [&]()
        {
            entry->name = arc_file.io.read_to_zero().str();
        });
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> FjsysArchiveDecoder::read_file(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    auto data = arc_file.io.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::Registry::add<FjsysArchiveDecoder>("nsystem/fjsys");
