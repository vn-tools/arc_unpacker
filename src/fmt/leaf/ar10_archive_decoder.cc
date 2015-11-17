#include "fmt/leaf/ar10_archive_decoder.h"
#include "util/encoding.h"
#include "util/range.h"

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

bool Ar10ArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.seek(0).read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Ar10ArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(magic.size());
    const auto file_count = arc_file.io.read_u32_le();
    const auto offset_to_data = arc_file.io.read_u32_le();
    auto meta = std::make_unique<ArchiveMetaImpl>();
    meta->archive_key = arc_file.io.read_u8();
    ArchiveEntryImpl *last_entry = nullptr;
    for (const auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = util::sjis_to_utf8(arc_file.io.read_to_zero()).str();
        entry->offset = arc_file.io.read_u32_le() + offset_to_data;
        if (last_entry)
            last_entry->size = entry->offset - last_entry->offset;
        last_entry = entry.get();
        meta->entries.push_back(std::move(entry));
    }
    if (last_entry)
        last_entry->size = arc_file.io.size() - last_entry->offset;
    return meta;
}

std::unique_ptr<File> Ar10ArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.io.seek(entry->offset);
    const auto data_size = arc_file.io.read_u32_le();
    const auto key_size = arc_file.io.read_u8() ^ meta->archive_key;
    const auto key = arc_file.io.read(key_size);
    auto data = arc_file.io.read(data_size);
    for (const auto i : util::range(data.size()))
        data[i] ^= key[i % key.size()];
    return std::make_unique<File>(entry->name, data);
}

std::vector<std::string> Ar10ArchiveDecoder::get_linked_formats() const
{
    return {"leaf/cz10"};
}

static auto dummy = fmt::register_fmt<Ar10ArchiveDecoder>("leaf/ar10");
