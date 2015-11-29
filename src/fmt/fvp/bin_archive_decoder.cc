#include "fmt/fvp/bin_archive_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::fvp;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool BinArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.name.has_extension("bin");
}

std::unique_ptr<fmt::ArchiveMeta>
    BinArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    size_t file_count = input_file.stream.read_u32_le();
    input_file.stream.skip(4);

    auto names_start = file_count * 12 + 8;
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        auto name_offset = input_file.stream.read_u32_le();
        input_file.stream.peek(names_start + name_offset, [&]()
        {
            entry->name = util::sjis_to_utf8(
                input_file.stream.read_to_zero()).str();
        });
        entry->offset = input_file.stream.read_u32_le();
        entry->size = input_file.stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> BinArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<io::File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> BinArchiveDecoder::get_linked_formats() const
{
    return {"fvp/nvsg"};
}

static auto dummy = fmt::register_fmt<BinArchiveDecoder>("fvp/bin");
