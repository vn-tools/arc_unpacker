#include "fmt/lilim/aos2_archive_decoder.h"
#include "algo/range.h"
#include "io/msb_bit_reader.h"

using namespace au;
using namespace au::fmt::lilim;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool Aos2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    if (input_file.stream.read_u32_le() != 0)
        return false;
    const auto data_offset = input_file.stream.read_u32_le();
    input_file.stream.seek(data_offset - 8);
    const auto last_entry_offset = input_file.stream.read_u32_le();
    const auto last_entry_size = input_file.stream.read_u32_le();
    const auto expected_size
        = data_offset + last_entry_offset + last_entry_size;
    return input_file.stream.size() == expected_size;
}

std::unique_ptr<fmt::ArchiveMeta> Aos2ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(4);
    const auto data_offset = input_file.stream.read_u32_le();
    const auto table_size = input_file.stream.read_u32_le();
    const auto file_count = table_size / 0x28;
    auto meta = std::make_unique<ArchiveMeta>();
    input_file.stream.seek(data_offset - table_size);
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = input_file.stream.read_to_zero(0x20).str();
        entry->offset = input_file.stream.read_u32_le() + data_offset;
        entry->size = input_file.stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> Aos2ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> Aos2ArchiveDecoder::get_linked_formats() const
{
    return {"lilim/scr", "lilim/abm", "microsoft/bmp"};
}

static auto dummy = fmt::register_fmt<Aos2ArchiveDecoder>("lilim/aos2");
