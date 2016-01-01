#include "dec/nsystem/fjsys_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::nsystem;

static const bstr magic = "FJSYS\x00\x00\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool FjsysArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> FjsysArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    auto header_size = input_file.stream.read_u32_le();
    auto file_names_size = input_file.stream.read_u32_le();
    auto file_names_start = header_size - file_names_size;
    auto file_count = input_file.stream.read_u32_le();
    input_file.stream.skip(64);

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        size_t file_name_offset = input_file.stream.read_u32_le();
        entry->size = input_file.stream.read_u32_le();
        entry->offset = input_file.stream.read_u64_le();
        input_file.stream.peek(file_name_offset + file_names_start, [&]()
        {
            entry->path = input_file.stream.read_to_zero().str();
        });
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> FjsysArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> FjsysArchiveDecoder::get_linked_formats() const
{
    return {"nsystem/mgd"};
}

static auto _ = dec::register_decoder<FjsysArchiveDecoder>("nsystem/fjsys");
