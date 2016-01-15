#include "dec/ism/isa_archive_decoder.h"
#include "algo/range.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::dec::ism;

static const bstr magic = "ISM ARCHIVED"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool IsaArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> IsaArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u16>();
    input_file.stream.skip(2);
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = input_file.stream.read_to_zero(12).str();
        input_file.stream.skip(4);
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        input_file.stream.skip(8);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> IsaArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<IsaArchiveDecoder>("ism/isa");
