#include "dec/cherry_soft/myk_archive_decoder.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::cherry_soft;

static const bstr magic = "MYK00\x1A\x00\x00"_b;

bool MykArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> MykArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u16>();
    const auto table_offset = input_file.stream.read_le<u32>();
    input_file.stream.seek(table_offset);

    auto meta = std::make_unique<ArchiveMeta>();
    auto current_offset = 16;
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->path = input_file.stream.read_to_zero(12).str();
        entry->offset = current_offset;
        entry->size = input_file.stream.read_le<u32>();
        current_offset += entry->size;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> MykArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

static auto _ = dec::register_decoder<MykArchiveDecoder>("cherry-soft/myk");
