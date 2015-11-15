#include "fmt/kiss/plg_archive_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::kiss;

static const bstr magic = "LIB\x00\x00\x00\x01\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool PlgArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    arc_file.io.seek(0);
    return arc_file.io.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    PlgArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    const auto file_count = arc_file.io.read_u32_le();
    arc_file.io.skip(4);
    auto meta = std::make_unique<ArchiveMeta>();
    for (const size_t i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.io.read_to_zero(0x20).str();
        if (arc_file.io.read_u32_le() != i)
            throw err::CorruptDataError("Unexpected entry index");
        entry->size = arc_file.io.read_u32_le();
        entry->offset = arc_file.io.read_u32_le();
        if (arc_file.io.read_u32_le() != 0)
            throw err::CorruptDataError("Expected '0'");
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> PlgArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    return std::make_unique<File>(entry->name, arc_file.io.read(entry->size));
}

std::vector<std::string> PlgArchiveDecoder::get_linked_formats() const
{
    return {"kiss/plg", "kiss/custom-png"};
}

static auto dummy = fmt::register_fmt<PlgArchiveDecoder>("kiss/plg");
