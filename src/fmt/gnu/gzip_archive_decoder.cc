#include "fmt/gnu/gzip_archive_decoder.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::gnu;

static const bstr magic = "\x1F\x8B"_b;

namespace
{
    struct Flags final
    {
        static const u8 Text     = 0b00000001;
        static const u8 Crc      = 0b00000010;
        static const u8 Extra    = 0b00000100;
        static const u8 FileName = 0b00001000;
        static const u8 Comment  = 0b00010000;
    };

    enum class CompressionMethod : u8
    {
        Deflate = 8,
    };

    enum class OperatingSystem : u8
    {
        FatFilesystem  = 0,
        Amiga          = 1,
        Vms            = 2,
        Unix           = 3,
        VmCms          = 4,
        AtariTos       = 5,
        HpfsFilesystem = 6,
        Macintosh      = 7,
        Zsystem        = 8,
        Cpm            = 9,
        Tops20         = 10,
        NtfsFilesystem = 11,
        Qdos           = 12,
        Acorn          = 13,
        Unknown        = 255,
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool GzipArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    arc_file.stream.seek(0);
    return arc_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    GzipArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.stream.seek(0);
    auto meta = std::make_unique<ArchiveMeta>();

    while (!arc_file.stream.eof())
    {
        arc_file.stream.skip(magic.size());
        const auto compression_method = static_cast<CompressionMethod>(
            arc_file.stream.read_u8());
        const auto flags = arc_file.stream.read_u8();
        const auto mtime = arc_file.stream.read_u32_le();
        const auto extra_flags = arc_file.stream.read_u8();
        const auto operation_system
            = static_cast<OperatingSystem>(arc_file.stream.read_u8());

        if (flags & Flags::Extra)
        {
            const auto extra_field_size = arc_file.stream.read_u16_le();
            arc_file.stream.skip(extra_field_size);
        }

        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->name = "";

        if (flags & Flags::FileName)
            entry->name = arc_file.stream.read_to_zero().str();

        if (flags & Flags::Comment)
            arc_file.stream.read_to_zero();

        if (flags & Flags::Crc)
            arc_file.stream.skip(2);

        entry->offset = arc_file.stream.tell();
        const auto data = util::pack::zlib_inflate(
            arc_file.stream, util::pack::ZlibKind::RawDeflate);
        entry->size = arc_file.stream.tell() - entry->offset;
        arc_file.stream.skip(8);

        meta->entries.push_back(std::move(entry));
    }

    return meta;
}

std::unique_ptr<File> GzipArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    arc_file.stream.seek(entry->offset);
    return std::make_unique<File>(
        entry->name,
        util::pack::zlib_inflate(
            arc_file.stream.read(entry->size),
            util::pack::ZlibKind::RawDeflate));
}

static auto dummy = fmt::register_fmt<GzipArchiveDecoder>("gnu/gzip");
