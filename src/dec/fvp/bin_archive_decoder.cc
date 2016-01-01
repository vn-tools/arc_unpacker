#include "dec/fvp/bin_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::fvp;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool BinArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("bin");
}

std::unique_ptr<dec::ArchiveMeta> BinArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    size_t file_count = input_file.stream.read_u32_le();
    input_file.stream.skip(4);

    auto names_start = file_count * 12 + 8;
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        auto name_offset = input_file.stream.read_u32_le();
        input_file.stream.peek(names_start + name_offset, [&]()
        {
            entry->path = algo::sjis_to_utf8(
                input_file.stream.read_to_zero()).str();
        });
        entry->offset = input_file.stream.read_u32_le();
        entry->size = input_file.stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> BinArchiveDecoder::read_file_impl(
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

std::vector<std::string> BinArchiveDecoder::get_linked_formats() const
{
    return {"fvp/nvsg"};
}

static auto _ = dec::register_decoder<BinArchiveDecoder>("fvp/bin");
