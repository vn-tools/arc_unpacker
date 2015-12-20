#include "fmt/leaf/ar10_group/ar10_archive_decoder.h"
#include "algo/locale.h"
#include "algo/range.h"

using namespace au;
using namespace au::fmt::leaf;

static const bstr magic = "ar10"_b;

namespace
{
    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        u8 archive_key;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool Ar10ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Ar10ArchiveDecoder::read_meta_impl(io::File &input_file) const
{
    input_file.stream.seek(magic.size());
    const auto file_count = input_file.stream.read_u32_le();
    const auto offset_to_data = input_file.stream.read_u32_le();
    auto meta = std::make_unique<ArchiveMetaImpl>();
    meta->archive_key = input_file.stream.read_u8();
    ArchiveEntryImpl *last_entry = nullptr;
    for (const auto i : algo::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = algo::sjis_to_utf8(
            input_file.stream.read_to_zero()).str();
        entry->offset = input_file.stream.read_u32_le() + offset_to_data;
        if (last_entry)
            last_entry->size = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }
    if (last_entry)
        last_entry->size = input_file.stream.size() - last_entry->offset;
    return std::move(meta);
}

std::unique_ptr<io::File> Ar10ArchiveDecoder::read_file_impl(
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);

    input_file.stream.seek(entry->offset);
    const auto data_size = input_file.stream.read_u32_le();
    const auto key_size = input_file.stream.read_u8() ^ meta->archive_key;
    const auto key = input_file.stream.read(key_size);
    auto data = input_file.stream.read(data_size);

    for (const auto i : algo::range(data.size()))
        data[i] ^= key[i % key.size()];

    auto output_file = std::make_unique<io::File>(entry->path, data);
    output_file->guess_extension();
    return output_file;
}

std::vector<std::string> Ar10ArchiveDecoder::get_linked_formats() const
{
    return {"leaf/cz10"};
}

static auto dummy = fmt::register_fmt<Ar10ArchiveDecoder>("leaf/ar10");
