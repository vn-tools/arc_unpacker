#include "dec/propeller/mpk_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::propeller;

namespace
{
    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool MpkArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.path.has_extension("mpk");
}

std::unique_ptr<dec::ArchiveMeta> MpkArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto table_offset = input_file.stream.read_le<u32>();
    auto file_count = input_file.stream.read_le<u32>();

    input_file.stream.seek(table_offset);
    auto meta = std::make_unique<ArchiveMeta>();
    for (auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();

        auto name_bin = input_file.stream.read(32);
        const u8 key8 = name_bin[31];
        const u32 key32 = (key8 << 24) | (key8 << 16) | (key8 << 8) | key8;

        for (auto &c : name_bin)
            c ^= key8;

        auto name = algo::sjis_to_utf8(name_bin).str(true);
        if (name[0] == '\\')
            name = name.substr(1);
        entry->path = name;

        entry->offset = input_file.stream.read_le<u32>() ^ key32;
        entry->size = input_file.stream.read_le<u32>() ^ key32;

        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<io::File> MpkArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    auto data = input_file.stream.read(entry->size);
    return std::make_unique<io::File>(entry->path, data);
}

std::vector<std::string> MpkArchiveDecoder::get_linked_formats() const
{
    return {"propeller/mgr"};
}

static auto _ = dec::register_decoder<MpkArchiveDecoder>("propeller/mpk");
