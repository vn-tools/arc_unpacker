#include "dec/alice_soft/alk_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::alice_soft;

static const bstr magic = "ALK0"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool AlkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> AlkArchiveDecoder::read_meta_impl(
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
        if (entry->size)
        {
            entry->path = algo::format("%03d.dat", meta->entries.size());
            meta->entries.push_back(std::move(entry));
        }
    }
    return meta;
}

std::unique_ptr<io::File> AlkArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> AlkArchiveDecoder::get_linked_formats() const
{
    return {"alice-soft/qnt"};
}

static auto _ = dec::register_decoder<AlkArchiveDecoder>("alice-soft/alk");
