#include "fmt/alice_soft/alk_archive_decoder.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::alice_soft;

static const bstr magic = "ALK0"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool AlkArchiveDecoder::is_recognized_impl(File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    AlkArchiveDecoder::read_meta_impl(File &input_file) const
{
    input_file.stream.seek(magic.size());
    auto file_count = input_file.stream.read_u32_le();
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->offset = input_file.stream.read_u32_le();
        entry->size = input_file.stream.read_u32_le();
        if (entry->size)
        {
            entry->name = util::format("%03d.dat", meta->entries.size());
            meta->entries.push_back(std::move(entry));
        }
    }
    return meta;
}

std::unique_ptr<File> AlkArchiveDecoder::read_file_impl(
    File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    auto output_file = std::make_unique<File>(entry->name, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> AlkArchiveDecoder::get_linked_formats() const
{
    return {"alice-soft/qnt"};
}

static auto dummy = fmt::register_fmt<AlkArchiveDecoder>("alice-soft/alk");
