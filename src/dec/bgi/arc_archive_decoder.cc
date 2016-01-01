#include "dec/bgi/arc_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::bgi;

static const bstr magic = "PackFile\x20\x20\x20\x20"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool ArcArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> ArcArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    auto file_count = input_file.stream.read_u32_le();
    auto file_data_start = input_file.stream.tell() + file_count * 32;
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = input_file.stream.read_to_zero(16).str();
        entry->offset = input_file.stream.read_u32_le() + file_data_start;
        entry->size = input_file.stream.read_u32_le();
        input_file.stream.skip(8);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> ArcArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> ArcArchiveDecoder::get_linked_formats() const
{
    return {"bgi/cbg", "bgi/dsc", "bgi/audio"};
}

static auto _ = dec::register_decoder<ArcArchiveDecoder>("bgi/arc");
