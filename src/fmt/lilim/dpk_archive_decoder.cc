#include "fmt/lilim/dpk_archive_decoder.h"
#include "algo/range.h"
#include "io/bit_reader.h"

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

bool DpkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    if (input_file.stream.read(magic.size()) != magic)
        return false;
    input_file.stream.skip(2);
    return input_file.stream.read_u32_le() == input_file.stream.size();
}

std::unique_ptr<fmt::ArchiveMeta>
    DpkArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_u16_le();
    input_file.stream.skip(4);
    auto meta = std::make_unique<ArchiveMeta>();
    size_t current_offset = magic.size() + 2 + 4 + file_count * 0x14;
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = input_file.stream.read_to_zero(0x10).str();
        entry->size = input_file.stream.read_u32_le();
        entry->offset = current_offset;
        current_offset += entry->size;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> DpkArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> DpkArchiveDecoder::get_linked_formats() const
{
    return {"lilim/dbm", "lilim/doj", "lilim/dwv"};
}

static auto dummy = fmt::register_fmt<DpkArchiveDecoder>("lilim/dpk");
