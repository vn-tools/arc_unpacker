#include "dec/kaguya/link_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::kaguya;

static const auto magic = "LINK"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool LinkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> LinkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    const auto file_names_size = input_file.stream.read_le<u32>();
    const auto file_names_start = input_file.stream.tell();

    std::vector<bstr> file_names;
    for (const auto i : algo::range(file_count))
        file_names.push_back(input_file.stream.read_to_zero());

    input_file.stream.seek(file_names_start + file_names_size);
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = file_names.at(i).str();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> LinkArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto output_file = std::make_unique<io::File>(
        entry->path,
        input_file.stream.seek(entry->offset).read(entry->size));
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> LinkArchiveDecoder::get_linked_formats() const
{
    return {"kaguya/compressed-bmp"};
}

static auto _ = dec::register_decoder<LinkArchiveDecoder>("kaguya/link");
