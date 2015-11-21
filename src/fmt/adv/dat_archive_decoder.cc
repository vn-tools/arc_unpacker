#include "fmt/adv/dat_archive_decoder.h"
#include "io/memory_stream.h"
#include "util/encoding.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::adv;

static const bstr magic = "ARCHIVE\x00"_b;

namespace
{
    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        bstr game_key;
        bstr arc_key;
        size_t file_count;
        size_t header_offset;
        size_t table_offset;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

static std::unique_ptr<ArchiveMetaImpl> prepare_meta(File &arc_file)
{
    auto meta = std::make_unique<ArchiveMetaImpl>();
    arc_file.stream.seek(53); // suspicious: varies for other games?
    meta->arc_key = arc_file.stream.read(8);

    arc_file.stream.skip(20);
    meta->header_offset = arc_file.stream.tell();
    bstr header_data = arc_file.stream.read(0x24);

    // recover game key
    meta->game_key.resize(8);
    for (const auto i : util::range(8))
        meta->game_key[i] = meta->arc_key[i] ^ header_data[i] ^ magic[i];

    for (const auto i : util::range(8))
        meta->arc_key[i] ^= meta->game_key[i];

    for (const auto i : util::range(0x24))
        header_data[i] ^= meta->arc_key[i % 8];

    meta->table_offset = header_data.get<const u32>()[4];
    meta->file_count = header_data.get<const u32>()[8];
    return meta;
}

bool DatArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    const auto meta = prepare_meta(arc_file);
    return meta->header_offset + meta->table_offset + 0x114 * meta->file_count
        == arc_file.stream.size();
}

std::unique_ptr<fmt::ArchiveMeta>
    DatArchiveDecoder::read_meta_impl(File &arc_file) const
{
    auto meta = prepare_meta(arc_file);

    arc_file.stream.seek(meta->header_offset + meta->table_offset);
    auto table_data = arc_file.stream.read(0x114 * meta->file_count);
    auto key_idx = meta->table_offset;
    for (const auto i : util::range(table_data.size()))
        table_data[i] ^= meta->arc_key[(key_idx++) % meta->arc_key.size()];
    io::MemoryStream table_stream(table_data);

    for (const auto i : util::range(meta->file_count))
    {
        table_stream.seek(0x114 * i);
        table_stream.skip(1);
        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = util::sjis_to_utf8(table_stream.read_to_zero()).str();
        table_stream.seek(0x114 * i + 0x108);
        entry->offset = table_stream.read_u32_le() + meta->header_offset;
        entry->size = table_stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return std::move(meta);
}

std::unique_ptr<File> DatArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.stream.seek(entry->offset);
    auto data = arc_file.stream.read(entry->size);
    auto key_idx = entry->offset - meta->header_offset;
    for (const auto i : util::range(data.size()))
        data[i] ^= meta->arc_key[key_idx++ % meta->arc_key.size()];
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::register_fmt<DatArchiveDecoder>("adv/dat");
