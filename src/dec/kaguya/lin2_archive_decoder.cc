#include "dec/kaguya/lin2_archive_decoder.h"
#include "algo/binary.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::kaguya;

static const auto magic = "LIN2"_b;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool Lin2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> Lin2ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_le<u32>();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        const auto name_size = input_file.stream.read_le<u16>();
        entry->path
            = algo::unxor(input_file.stream.read(name_size), 0xFF).c_str();
        entry->offset = input_file.stream.read_le<u32>();
        entry->size = input_file.stream.read_le<u32>();
        input_file.stream.skip(2);
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Lin2ArchiveDecoder::read_file_impl(
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

std::vector<std::string> Lin2ArchiveDecoder::get_linked_formats() const
{
    return {"kaguya/compressed-bmp"};
}

static auto _ = dec::register_decoder<Lin2ArchiveDecoder>("kaguya/lin2");
