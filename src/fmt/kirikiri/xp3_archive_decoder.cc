#include "fmt/kirikiri/xp3_archive_decoder.h"
#include "err.h"
#include "fmt/kirikiri/xp3_filter_registry.h"
#include "io/buffered_io.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/range.h"

using namespace au;
using namespace au::fmt::kirikiri;

namespace
{
    struct InfoChunk final
    {
        u32 flags;
        u64 file_size_orig;
        u64 file_size_comp;
        std::string name;
    };

    struct SegmChunk final
    {
        u32 flags;
        u64 offset;
        u64 size_orig;
        u64 size_comp;
    };

    struct AdlrChunk final
    {
        u32 key;
    };

    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        std::unique_ptr<Xp3Filter> filter;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        std::vector<SegmChunk> segm_chunks;
        AdlrChunk adlr_chunk;
    };
}

static const bstr xp3_magic = "XP3\r\n\x20\x0A\x1A\x8B\x67\x01"_b;
static const bstr file_magic = "File"_b;
static const bstr adlr_magic = "adlr"_b;
static const bstr info_magic = "info"_b;
static const bstr segm_magic = "segm"_b;

static int detect_version(io::IO &arc_io)
{
    int version = 1;
    size_t old_pos = arc_io.tell();
    arc_io.seek(19);
    if (arc_io.read_u32_le() == 1)
        version = 2;
    arc_io.seek(old_pos);
    return version;
}

static u64 get_table_offset(io::IO &arc_io, int version)
{
    if (version == 1)
        return arc_io.read_u64_le();

    u64 additional_header_offset = arc_io.read_u64_le();
    u32 minor_version = arc_io.read_u32_le();
    if (minor_version != 1)
        throw err::CorruptDataError("Unexpected XP3 version");

    arc_io.seek(additional_header_offset);
    arc_io.skip(1); // flags?
    arc_io.skip(8); // table size
    return arc_io.read_u64_le();
}

static InfoChunk read_info_chunk(io::IO &table_io)
{
    if (table_io.read(info_magic.size()) != info_magic)
        throw err::CorruptDataError("Expected INFO chunk");
    u64 info_chunk_size = table_io.read_u64_le();

    InfoChunk info_chunk;
    info_chunk.flags = table_io.read_u32_le();
    info_chunk.file_size_orig = table_io.read_u64_le();
    info_chunk.file_size_comp = table_io.read_u64_le();

    size_t file_name_size = table_io.read_u16_le();
    auto name = table_io.read(file_name_size * 2);
    info_chunk.name = util::convert_encoding(name, "utf-16le", "utf-8").str();
    return info_chunk;
}

static std::vector<SegmChunk> read_segm_chunks(io::IO &table_io)
{
    if (table_io.read(segm_magic.size()) != segm_magic)
        throw err::CorruptDataError("Expected SEGM chunk");

    auto segm_chunk_size = table_io.read_u64_le();
    if (segm_chunk_size % 28 != 0)
        throw err::CorruptDataError("Unexpected SEGM chunk size");

    std::vector<SegmChunk> segm_chunks;
    for (auto i : util::range(segm_chunk_size / 28))
    {
        SegmChunk segm_chunk;
        segm_chunk.flags = table_io.read_u32_le();
        segm_chunk.offset = table_io.read_u64_le();
        segm_chunk.size_orig = table_io.read_u64_le();
        segm_chunk.size_comp = table_io.read_u64_le();
        segm_chunks.push_back(segm_chunk);
    }
    return segm_chunks;
}

static AdlrChunk read_adlr_chunk(io::IO &table_io)
{
    if (table_io.read(adlr_magic.size()) != adlr_magic)
        throw err::CorruptDataError("Expected ADLR chunk");

    u64 adlr_chunk_size = table_io.read_u64_le();
    if (adlr_chunk_size != 4)
        throw err::CorruptDataError("Unexpected ADLR chunk size");

    AdlrChunk adlr_chunk;
    adlr_chunk.key = table_io.read_u32_le();
    return adlr_chunk;
}

struct Xp3ArchiveDecoder::Priv final
{
    Xp3FilterRegistry filter_registry;
};

Xp3ArchiveDecoder::Xp3ArchiveDecoder() : p(new Priv)
{
}

Xp3ArchiveDecoder::~Xp3ArchiveDecoder()
{
}

void Xp3ArchiveDecoder::register_cli_options(ArgParser &arg_parser) const
{
    p->filter_registry.register_cli_options(arg_parser);
    ArchiveDecoder::register_cli_options(arg_parser);
}

void Xp3ArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    p->filter_registry.parse_cli_options(arg_parser);
    ArchiveDecoder::parse_cli_options(arg_parser);
}

bool Xp3ArchiveDecoder::is_recognized_impl(File &arc_file) const
{
    return arc_file.io.read(xp3_magic.size()) == xp3_magic;
}

std::unique_ptr<fmt::ArchiveMeta>
    Xp3ArchiveDecoder::read_meta_impl(File &arc_file) const
{
    arc_file.io.seek(xp3_magic.size());
    auto version = detect_version(arc_file.io);
    auto table_offset = get_table_offset(arc_file.io, version);

    arc_file.io.seek(table_offset);
    auto table_is_compressed = arc_file.io.read_u8() != 0;
    auto table_size_comp = arc_file.io.read_u64_le();
    auto table_size_orig = table_is_compressed
        ? arc_file.io.read_u64_le()
        : table_size_comp;
    auto table_data = arc_file.io.read(table_size_comp);
    if (table_is_compressed)
        table_data = util::pack::zlib_inflate(table_data);
    io::BufferedIO table_io(table_data);

    auto meta = std::make_unique<ArchiveMetaImpl>();
    meta->filter.reset(new Xp3Filter(arc_file.name));
    p->filter_registry.set_decoder(*meta->filter);

    while (table_io.tell() < table_io.size())
    {
        auto entry = std::make_unique<ArchiveEntryImpl>();
        if (table_io.read(file_magic.size()) != file_magic)
            throw err::CorruptDataError("Expected FILE chunk");

        auto file_chunk_size = table_io.read_u64_le();
        auto file_chunk_start_offset = table_io.tell();

        auto info_chunk = read_info_chunk(table_io);
        entry->segm_chunks = read_segm_chunks(table_io);
        entry->adlr_chunk = read_adlr_chunk(table_io);

        if (table_io.tell() - file_chunk_start_offset != file_chunk_size)
            throw err::CorruptDataError("Unexpected file data size");

        entry->name = info_chunk.name;
        meta->entries.push_back(std::move(entry));
    }
    return meta;
}

std::unique_ptr<File> Xp3ArchiveDecoder::read_file_impl(
    File &arc_file, const ArchiveMeta &m, const ArchiveEntry &e) const
{
    auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);

    bstr data;
    for (auto &segm_chunk : entry->segm_chunks)
    {
        arc_file.io.seek(segm_chunk.offset);
        bool data_is_compressed = (segm_chunk.flags & 7) > 0;
        data += data_is_compressed
            ? util::pack::zlib_inflate(arc_file.io.read(segm_chunk.size_comp))
            : arc_file.io.read(segm_chunk.size_orig);
    }

    if (meta->filter && meta->filter->decoder)
        meta->filter->decoder(data, entry->adlr_chunk.key);

    return std::make_unique<File>(entry->name, data);
}

void Xp3ArchiveDecoder::set_plugin(const std::string &plugin_name)
{
    p->filter_registry.use_plugin(plugin_name);
}

std::vector<std::string> Xp3ArchiveDecoder::get_linked_formats() const
{
    return { "kirikiri/tlg" };
}

static auto dummy = fmt::register_fmt<Xp3ArchiveDecoder>("kirikiri/xp3");
