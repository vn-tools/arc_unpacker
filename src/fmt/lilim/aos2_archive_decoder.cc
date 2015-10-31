#include "fmt/lilim/aos2_archive_decoder.h"
#include "io/bit_reader.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::lilim;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool Aos2ArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    arc_file.io.seek(0);
    if (arc_file.io.read_u32_le() != 0)
        return false;
    const auto data_offset = arc_file.io.read_u32_le();
    arc_file.io.seek(data_offset - 8);
    const auto last_entry_offset = arc_file.io.read_u32_le();
    const auto last_entry_size = arc_file.io.read_u32_le();
    const auto expected_size
        = data_offset + last_entry_offset + last_entry_size;
    return arc_file.io.size() == expected_size;
}

std::unique_ptr<fmt::ArchiveMeta>
    Aos2ArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(4);
    const auto data_offset = arc_file.io.read_u32_le();
    const auto table_size = arc_file.io.read_u32_le();
    const auto file_count = table_size / 0x28;
    auto meta = std::make_unique<ArchiveMeta>();
    arc_file.io.seek(data_offset - table_size);
    for (const auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.io.read_to_zero(0x20).str();
        entry->offset = arc_file.io.read_u32_le() + data_offset;
        entry->size = arc_file.io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> Aos2ArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    return std::make_unique<File>(entry->name, arc_file.io.read(entry->size));
}

std::vector<std::string> Aos2ArchiveDecoder::get_linked_formats() const
{
    return { "lilim/scr", "lilim/abm", "microsoft/bmp" };
}

static auto dummy = fmt::register_fmt<Aos2ArchiveDecoder>("lilim/aos2");
