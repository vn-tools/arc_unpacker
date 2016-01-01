#include "dec/gnu/gzip_archive_decoder.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"

using namespace au;
using namespace au::dec::gnu;

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

    struct ArchiveEntryImpl final : dec::ArchiveEntry
    {
        size_t offset;
        size_t size;
    };
}

bool GzipArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    input_file.stream.seek(0);
    return input_file.stream.read(magic.size()) == magic;
}

std::unique_ptr<dec::ArchiveMeta> GzipArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    input_file.stream.seek(0);
    auto meta = std::make_unique<ArchiveMeta>();

    while (!input_file.stream.eof())
    {
        input_file.stream.skip(magic.size());
        const auto compression_method = static_cast<CompressionMethod>(
            input_file.stream.read_u8());
        const auto flags = input_file.stream.read_u8();
        const auto mtime = input_file.stream.read_u32_le();
        const auto extra_flags = input_file.stream.read_u8();
        const auto operation_system
            = static_cast<OperatingSystem>(input_file.stream.read_u8());

        if (flags & Flags::Extra)
        {
            const auto extra_field_size = input_file.stream.read_u16_le();
            input_file.stream.skip(extra_field_size);
        }

        auto entry = std::make_unique<ArchiveEntryImpl>();
        entry->path = "";

        if (flags & Flags::FileName)
            entry->path = input_file.stream.read_to_zero().str();

        if (flags & Flags::Comment)
            input_file.stream.read_to_zero();

        if (flags & Flags::Crc)
            input_file.stream.skip(2);

        entry->offset = input_file.stream.tell();
        const auto data = algo::pack::zlib_inflate(
            input_file.stream, algo::pack::ZlibKind::RawDeflate);
        entry->size = input_file.stream.tell() - entry->offset;
        input_file.stream.skip(8);

        meta->entries.push_back(std::move(entry));
    }

    return meta;
}

std::unique_ptr<io::File> GzipArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const dec::ArchiveMeta &m,
    const dec::ArchiveEntry &e) const
{
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    input_file.stream.seek(entry->offset);
    return std::make_unique<io::File>(
        entry->path,
        algo::pack::zlib_inflate(
            input_file.stream.read(entry->size),
            algo::pack::ZlibKind::RawDeflate));
}

static auto _ = dec::register_decoder<GzipArchiveDecoder>("gnu/gzip");
