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

bool PlgArchiveDecoder::is_recognized_impl(File &input_file) const
{
    input_file.stream.seek(0);
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    PlgArchiveDecoder::read_meta_impl(File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_u32_le();
    input_file.stream.skip(4);
    auto meta = std::make_unique<ArchiveMeta>();
    for (const size_t i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = input_file.stream.read_to_zero(0x20).str();
        if (input_file.stream.read_u32_le() != i)
            throw err::CorruptDataError("Unexpected entry index");
        entry->size = input_file.stream.read_u32_le();
        entry->offset = input_file.stream.read_u32_le();
        if (input_file.stream.read_u32_le() != 0)
            throw err::CorruptDataError("Expected '0'");
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> PlgArchiveDecoder::read_file_impl(
    File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> PlgArchiveDecoder::get_linked_formats() const
{
    return {"kiss/plg", "kiss/custom-png"};
}

static auto dummy = fmt::register_fmt<PlgArchiveDecoder>("kiss/plg");
