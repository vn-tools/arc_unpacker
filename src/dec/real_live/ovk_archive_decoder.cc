#include "dec/real_live/ovk_archive_decoder.h"
#include "algo/format.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::real_live;

bool OvkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("ovk");
}

std::unique_ptr<dec::ArchiveMeta> OvkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto file_count = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<PlainArchiveEntry>();
        entry->size = input_file.stream.read_le<u32>();
        entry->offset = input_file.stream.read_le<u32>();
        const auto file_id = input_file.stream.read_le<u32>();
        entry->path = algo::format("sample%05d", file_id);
        input_file.stream.skip(4);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> OvkArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const PlainArchiveEntry*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

static auto _ = dec::register_decoder<OvkArchiveDecoder>("real-live/ovk");
