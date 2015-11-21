#include "fmt/yuka_script/ykc_archive_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::yuka_script;

static const bstr magic = "YKC001"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t size;
        size_t offset;
    };
}

bool YkcArchiveDecoder::is_recognized_impl(File &input_file) const
{
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    YkcArchiveDecoder::read_meta_impl(File &input_file) const
{
    input_file.stream.seek(magic.size() + 10);
    size_t table_offset = input_file.stream.read_u32_le();
    size_t table_size = input_file.stream.read_u32_le();
    size_t file_count = table_size / 20;

    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();

        input_file.stream.seek(table_offset + i * 20);
        size_t name_origin = input_file.stream.read_u32_le();
        size_t name_size = input_file.stream.read_u32_le();
        entry->offset = input_file.stream.read_u32_le();
        entry->size = input_file.stream.read_u32_le();
        input_file.stream.skip(4);

        input_file.stream.seek(name_origin);
        entry->name = util::sjis_to_utf8(
            input_file.stream.read_to_zero(name_size)).str();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> YkcArchiveDecoder::read_file_impl(
    File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> YkcArchiveDecoder::get_linked_formats() const
{
    return {"yuka-script/ykg"};
}

static auto dummy = fmt::register_fmt<YkcArchiveDecoder>("yuka-script/ykc");
