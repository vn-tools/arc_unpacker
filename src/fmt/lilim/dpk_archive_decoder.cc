#include "fmt/lilim/dpk_archive_decoder.h"
#include "io/bit_reader.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::lilim;

static const bstr magic = "PA"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool DpkArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    arc_file.io.seek(0);
    if (arc_file.io.read(magic.size()) != magic)
        return false;
    arc_file.io.skip(2);
    return arc_file.io.read_u32_le() == arc_file.io.size();
}

std::unique_ptr<fmt::ArchiveMeta>
    DpkArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    const auto file_count = arc_file.io.read_u16_le();
    arc_file.io.skip(4);
    auto meta = std::make_unique<ArchiveMeta>();
    size_t current_offset = magic.size() + 2 + 4 + file_count * 0x14;
    for (const auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.io.read_to_zero(0x10).str();
        entry->size = arc_file.io.read_u32_le();
        entry->offset = current_offset;
        current_offset += entry->size;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> DpkArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    return std::make_unique<File>(entry->name, arc_file.io.read(entry->size));
}

std::vector<std::string> DpkArchiveDecoder::get_linked_formats() const
{
    return { "lilim/dbm", "lilim/doj", "lilim/dwv" };
}

static auto dummy = fmt::register_fmt<DpkArchiveDecoder>("lilim/dpk");
