#include "dec/cri/afs_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::cri;

static const bstr magic = "AFS\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool AfsArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> AfsArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    const auto names_offset = input_file.stream.read_le<u32>();
    input_file.stream.seek(names_offset);
    for (const auto i : algo::range(file_count))
    {
        meta->entries[i]->path = input_file.stream.read_to_zero(32).str();
        input_file.stream.skip(16);
    }
    return meta;
}

std::unique_ptr<io::File> AfsArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    const auto data = input_file.stream.read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> AfsArchiveDecoder::get_linked_formats() const
{
    return {"cri/afs"};
}

static auto _ = dec::register_decoder<AfsArchiveDecoder>("cri/afs");
