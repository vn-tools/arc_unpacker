#include "fmt/playstation/gpda_archive_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::playstation;

static const bstr magic = "GPDA"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool GpdaArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    arc_file.stream.seek(0);
    return arc_file.stream.read(magic.size()) == magic
        && arc_file.stream.read_u32_le() == arc_file.stream.size();
}

std::unique_ptr<fmt::ArchiveMeta>
    GpdaArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.stream.seek(12);
    const auto file_count = arc_file.stream.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();

        entry->offset = arc_file.stream.read_u32_le();
        if (arc_file.stream.read_u32_le() != 0)
            throw err::CorruptDataError("Expected '0'");
        entry->size = arc_file.stream.read_u32_le();

        const auto name_offset = arc_file.stream.read_u32_le();
        if (!name_offset)
            entry->name = "";
        else
            arc_file.stream.peek(name_offset, [&]()
                {
                    const auto name_size = arc_file.stream.read_u32_le();
                    entry->name = arc_file.stream.read(name_size).str();
                });

        if (!entry->size)
            continue;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> GpdaArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = arc_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> GpdaArchiveDecoder::get_linked_formats() const
{
    return {"playstation/gpda", "playstation/gim", "cri/hca", "gnu/gzip"};
}

static auto dummy = fmt::register_fmt<GpdaArchiveDecoder>("playstation/gpda");
