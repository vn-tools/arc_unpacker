#include "dec/aoi/vfs_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::aoi;

static const bstr magic = "VF\x00\x01"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool VfsArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> VfsArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(4);
    const auto file_count = input_file.stream.read_le<u16>();
    const auto entry_size = input_file.stream.read_le<u16>();
    const auto table_size = input_file.stream.read_le<u32>();
    input_file.stream.skip(4);
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        const auto entry_offset = input_file.stream.tell();
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = input_file.stream.read_to_zero(0x13).str();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
        input_file.stream.seek(entry_offset + entry_size);
    }
    return meta;
}

std::unique_ptr<io::File> VfsArchiveDecoder::read_file_impl(
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

std::vector<std::string> VfsArchiveDecoder::get_linked_formats() const
{
    return {"aoi/iph", "aoi/aog"};
}

static auto _ = dec::register_decoder<VfsArchiveDecoder>("aoi/vfs");
