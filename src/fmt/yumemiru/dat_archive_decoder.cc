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

bool DatArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    if (input_file.stream.read(magic1.size()) == magic1)
        return true;
    input_file.stream.seek(0);
    return input_file.stream.read(magic2.size()) == magic2;
}

std::unique_ptr<fmt::ArchiveMeta>
    DatArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(8);
    const auto file_count = input_file.stream.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = input_file.stream.read_to_zero(0x100).str();
        entry->offset = input_file.stream.read_u32_le();
        entry->size_orig = input_file.stream.read_u32_le();
        entry->size_comp = input_file.stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> DatArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    if (entry->size_orig != entry->size_comp)
        throw err::NotSupportedError("Compressed archives are not supported");
    auto data = input_file.stream.read(entry->size_comp);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> DatArchiveDecoder::get_linked_formats() const
{
    return {"yumemiru/epf"};
}

static auto dummy = fmt::register_fmt<DatArchiveDecoder>("yumemiru/dat");
