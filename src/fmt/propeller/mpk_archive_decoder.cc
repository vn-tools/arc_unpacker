#include "fmt/propeller/mpk_archive_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::propeller;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool MpkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.has_extension("mpk");
}

std::unique_ptr<fmt::ArchiveMeta>
    MpkArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    auto table_offset = input_file.stream.read_u32_le();
    auto file_count = input_file.stream.read_u32_le();

    input_file.stream.seek(table_offset);
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();

        auto name = input_file.stream.read(32);
        u8 key8 = name[31];
        u32 key32 = (key8 << 24) | (key8 << 16) | (key8 << 8) | key8;

        for (auto i : util::range(32))
            name[i] ^= key8;

        entry->name = util::sjis_to_utf8(name).str();
        if (entry->name[0] == '\\')
            entry->name = entry->name.substr(1);
        entry->name.erase(entry->name.find('\x00'));

        entry->offset = input_file.stream.read_u32_le() ^ key32;
        entry->size = input_file.stream.read_u32_le() ^ key32;

        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> MpkArchiveDecoder::read_file_impl(
    io::File &input_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    return std::make_unique<io::File>(entry->name, data);
}

std::vector<std::string> MpkArchiveDecoder::get_linked_formats() const
{
    return {"propeller/mgr"};
}

static auto dummy = fmt::register_fmt<MpkArchiveDecoder>("propeller/mpk");
