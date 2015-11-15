#include "fmt/yumemiru/dat_archive_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::yumemiru;

static const bstr magic1 = "yanepkDx"_b;
static const bstr magic2 = "PackDat3"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size_orig;
        size_t size_comp;
    };
}

bool DatArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    arc_file.io.seek(0);
    if (arc_file.io.read(magic1.size()) == magic1)
        return true;
    arc_file.io.seek(0);
    return arc_file.io.read(magic2.size()) == magic2;
}

std::unique_ptr<fmt::ArchiveMeta>
    DatArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(8);
    const auto file_count = arc_file.io.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = arc_file.io.read_to_zero(0x100).str();
        entry->offset = arc_file.io.read_u32_le();
        entry->size_orig = arc_file.io.read_u32_le();
        entry->size_comp = arc_file.io.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> DatArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    if (entry->size_orig != entry->size_comp)
        throw err::NotSupportedError("Compressed archives are not supported");
    auto data = arc_file.io.read(entry->size_comp);
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> DatArchiveDecoder::get_linked_formats() const
{
    return {"yumemiru/epf"};
}

static auto dummy = fmt::register_fmt<DatArchiveDecoder>("yumemiru/dat");
