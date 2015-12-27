#include "fmt/crowd/pck_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"

using namespace au;
using namespace au::fmt::crowd;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool PckArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    if (!input_file.path.has_extension("pck"))
        return false;
    const auto file_count = input_file.stream.seek(0).read_u32_le();
    input_file.stream.skip(file_count * 12 - 8);
    const auto last_file_offset = input_file.stream.read_u32_le();
    const auto last_file_size = input_file.stream.read_u32_le();
    return last_file_offset + last_file_size == input_file.stream.size();
}

std::unique_ptr<fmt::ArchiveMeta> PckArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    const auto file_count = input_file.stream.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        input_file.stream.skip(4);
        entry->offset = input_file.stream.read_u32_le();
        entry->size = input_file.stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    for (auto &entry : meta->entries)
    {
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read_to_zero()).str();
    }
    return meta;
}

std::unique_ptr<io::File> PckArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    const auto data = input_file.stream.seek(entry->offset).read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> PckArchiveDecoder::get_linked_formats() const
{
    return {"crowd/eog"};
}

static auto dummy = fmt::register_fmt<PckArchiveDecoder>("crowd/pck");
