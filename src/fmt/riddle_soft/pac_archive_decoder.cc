#include "fmt/riddle_soft/pac_archive_decoder.h"
#include "err.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::riddle_soft;

static const bstr magic = "PAC1"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool PacArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    PacArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    auto file_count = input_file.stream.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    auto file_data_start = input_file.stream.tell() + file_count * 32;
    auto current_file_offset = 0;
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = input_file.stream.read_to_zero(16).str();
        entry->size = input_file.stream.read_u32_le();
        auto prefix = input_file.stream.read(4);
        auto unk1 = input_file.stream.read_u32_le();
        auto unk2 = input_file.stream.read_u32_le();
        if (unk1 != unk2)
            throw err::CorruptDataError("Data mismatch");
        entry->offset = file_data_start + current_file_offset;
        current_file_offset += entry->size;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> PacArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    return std::make_unique<io::File>(entry->name, data);
}

std::vector<std::string> PacArchiveDecoder::get_linked_formats() const
{
    return {"riddle-soft/cmp"};
}

static auto dummy = fmt::register_fmt<PacArchiveDecoder>("riddle-soft/pac");
