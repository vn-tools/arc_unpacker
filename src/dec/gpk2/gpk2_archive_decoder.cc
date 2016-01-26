#include "dec/gpk2/gpk2_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::gpk2;

static const bstr magic = "GPK2"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool Gpk2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Gpk2ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto table_offset = input_file.stream.read_le<u32>();
    input_file.stream.seek(table_offset);
    const auto file_count = input_file.stream.read_le<u32>();

    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        entry->path = input_file.stream.read_to_zero(0x80).str();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Gpk2ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    return std::make_unique<io::File>(
        entry->path,
        input_file.stream.seek(entry->offset).read(entry->size));
}

std::vector<std::string> Gpk2ArchiveDecoder::get_linked_formats() const
{
    return {"gpk2/gfb"};
}

static auto _ = dec::register_decoder<Gpk2ArchiveDecoder>("gpk2/gpk2");
