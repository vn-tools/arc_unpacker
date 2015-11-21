#include "fmt/nitroplus/pak_archive_decoder.h"
#include "io/memory_stream.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nitroplus;

static const bstr magic = "\x02\x00\x00\x00"_b;

namespace
{
    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        bool compressed;
        size_t offset;
        size_t size_orig;
        size_t size_comp;
    };
}

bool PakArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    if (arc_file.stream.read(magic.size()) != magic)
        return false;
    return read_meta(arc_file)->entries.size() > 0;
}

std::unique_ptr<fmt::ArchiveMeta>
    PakArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.stream.seek(magic.size());
    auto file_count = arc_file.stream.read_u32_le();
    auto table_size_orig = arc_file.stream.read_u32_le();
    auto table_size_comp = arc_file.stream.read_u32_le();
    arc_file.stream.skip(0x104);

    io::MemoryStream table_stream(
        util::pack::zlib_inflate(
            arc_file.stream.read(table_size_comp)));

    auto meta = std::make_unique<ArchiveMeta>();
    auto file_data_offset = arc_file.stream.tell();
    for (auto i : util::range(file_count))
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        size_t file_name_size = table_stream.read_u32_le();
        entry->name = util::sjis_to_utf8(
            table_stream.read(file_name_size)).str();
        entry->offset = table_stream.read_u32_le() + file_data_offset;
        entry->size_orig = table_stream.read_u32_le();
        table_stream.skip(4);
        entry->compressed = table_stream.read_u32_le() > 0;
        entry->size_comp = table_stream.read_u32_le();
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> PakArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.stream.seek(entry->offset);
    auto data = entry->compressed
        ? util::pack::zlib_inflate(arc_file.stream.read(entry->size_comp))
        : arc_file.stream.read(entry->size_orig);
    return std::make_unique<File>(entry->name, data);
}

static auto dummy = fmt::register_fmt<PakArchiveDecoder>("nitroplus/pak");
