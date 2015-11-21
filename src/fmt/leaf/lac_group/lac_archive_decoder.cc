#include "fmt/leaf/lac_group/lac_archive_decoder.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "LAC\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool LacArchiveDecoder::is_recognized_impl(File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    LacArchiveDecoder::read_meta_impl(File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_u32_le();

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = input_file.stream.read_to_zero(32).str();
        for (auto &c : entry->name)
            c ^= 0xFF;
        entry->size = input_file.stream.read_u32_le();
        entry->offset = input_file.stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> LacArchiveDecoder::read_file_impl(
    File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    const auto data = input_file.stream.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::register_fmt<LacArchiveDecoder>("leaf/lac");
