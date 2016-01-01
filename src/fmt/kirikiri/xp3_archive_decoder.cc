#include "fmt/kirikiri/xp3_archive_decoder.h"
#include "algo/locale.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "err.h"
#include "fmt/kirikiri/xp3_filter_registry.h"
#include "io/memory_stream.h"

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

    struct TimeChunk final
    {
        u64 timestamp;
    };

    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        std::unique_ptr<Xp3Filter> filter;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        std::unique_ptr<InfoChunk> info_chunk;
        std::vector<std::unique_ptr<SegmChunk>> segm_chunks;
        std::unique_ptr<AdlrChunk> adlr_chunk;
        std::unique_ptr<TimeChunk> time_chunk;
    };
}

static const bstr xp3_magic = "XP3\r\n\x20\x0A\x1A\x8B\x67\x01"_b;
static const bstr file_entry_magic = "File"_b;
static const bstr info_chunk_magic = "info"_b;
static const bstr segm_chunk_magic = "segm"_b;
static const bstr adlr_chunk_magic = "adlr"_b;
static const bstr time_chunk_magic = "time"_b;

static int detect_version(io::IStream &input_stream)
{
    if (input_stream.seek(19).read_u32_le() == 1)
        return 2;
    return 1;
}

static u64 get_table_offset(io::IStream &input_stream, int version)
{
    input_stream.seek(xp3_magic.size());
    if (version == 1)
        return input_stream.read_u64_le();

    u64 additional_header_offset = input_stream.read_u64_le();
    u32 minor_version = input_stream.read_u32_le();
    if (minor_version != 1)
        throw err::CorruptDataError("Unexpected XP3 version");

    input_stream.seek(additional_header_offset);
    input_stream.skip(1); // flags?
    input_stream.skip(8); // table size
    return input_stream.read_u64_le();
}

static std::unique_ptr<InfoChunk> read_info_chunk(io::IStream &chunk_stream)
{
    auto info_chunk = std::make_unique<InfoChunk>();
    info_chunk->flags = chunk_stream.read_u32_le();
    info_chunk->file_size_orig = chunk_stream.read_u64_le();
    info_chunk->file_size_comp = chunk_stream.read_u64_le();

    const auto file_name_size = chunk_stream.read_u16_le();
    const auto name = chunk_stream.read(file_name_size * 2);
    info_chunk->name = algo::utf16_to_utf8(name).str();
    return info_chunk;
}

static std::vector<std::unique_ptr<SegmChunk>> read_segm_chunks(
    io::IStream &chunk_stream)
{
    std::vector<std::unique_ptr<SegmChunk>> segm_chunks;
    while (!chunk_stream.eof())
    {
        auto segm_chunk = std::make_unique<SegmChunk>();
        segm_chunk->flags = chunk_stream.read_u32_le();
        segm_chunk->offset = chunk_stream.read_u64_le();
        segm_chunk->size_orig = chunk_stream.read_u64_le();
        segm_chunk->size_comp = chunk_stream.read_u64_le();
        segm_chunks.push_back(std::move(segm_chunk));
    }
    return segm_chunks;
}

static std::unique_ptr<AdlrChunk> read_adlr_chunk(io::IStream &chunk_stream)
{
    auto adlr_chunk = std::make_unique<AdlrChunk>();
    adlr_chunk->key = chunk_stream.read_u32_le();
    return adlr_chunk;
}

static std::unique_ptr<TimeChunk> read_time_chunk(io::IStream &chunk_stream)
{
    auto time_chunk = std::make_unique<TimeChunk>();
    time_chunk->timestamp = chunk_stream.read_u64_le();
    return time_chunk;
}

static std::unique_ptr<ArchiveEntryImpl> read_entry(
    const Logger &logger, io::IStream &input_stream)
{
    if (input_stream.read(file_entry_magic.size()) != file_entry_magic)
        throw err::CorruptDataError("Expected FILE entry");

    const auto entry_size = input_stream.read_u64_le();
    io::MemoryStream entry_stream(input_stream.read(entry_size));

    auto entry = std::make_unique<ArchiveEntryImpl>();
    while (!entry_stream.eof())
    {
        const auto chunk_magic = entry_stream.read(4);
        const auto chunk_size = entry_stream.read_u64_le();
        io::MemoryStream chunk_stream(entry_stream.read(chunk_size));

        if (chunk_magic == info_chunk_magic)
            entry->info_chunk = read_info_chunk(chunk_stream);
        else if (chunk_magic == segm_chunk_magic)
            entry->segm_chunks = read_segm_chunks(chunk_stream);
        else if (chunk_magic == adlr_chunk_magic)
            entry->adlr_chunk = read_adlr_chunk(chunk_stream);
        else if (chunk_magic == time_chunk_magic)
            entry->time_chunk = read_time_chunk(chunk_stream);
        else
        {
            logger.warn("Unknown chunk '%s'", chunk_magic.get<char>());
            continue;
        }

        if (!chunk_stream.eof())
        {
            logger.warn(
                "'%s' chunk contains data beyond EOF\n",
                chunk_magic.get<char>());
        }
    }
    if (!entry_stream.eof())
        throw err::CorruptDataError("FILE entry contains data beyond EOF");

    if (!entry->info_chunk)
        throw err::CorruptDataError("INFO chunk not found");
    if (!entry->adlr_chunk)
        throw err::CorruptDataError("ADLR chunk not found");
    if (entry->segm_chunks.empty())
        throw err::CorruptDataError("No SEGM chunks found");

    entry->path = entry->info_chunk->name;
    return entry;
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
}

void Xp3ArchiveDecoder::parse_cli_options(const ArgParser &arg_parser)
{
    p->filter_registry.parse_cli_options(arg_parser);
}

bool Xp3ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return input_file.stream.read(xp3_magic.size()) == xp3_magic;
}

std::unique_ptr<fmt::ArchiveMeta> Xp3ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    const auto version = detect_version(input_file.stream);
    const auto table_offset = get_table_offset(input_file.stream, version);

    input_file.stream.seek(table_offset);
    const auto table_is_compressed = input_file.stream.read_u8() != 0;
    const auto table_size_comp = input_file.stream.read_u64_le();
    const auto table_size_orig = table_is_compressed
        ? input_file.stream.read_u64_le()
        : table_size_comp;

    auto table_data = input_file.stream.read(table_size_comp);
    if (table_is_compressed)
        table_data = algo::pack::zlib_inflate(table_data);
    io::MemoryStream table_stream(table_data);

    auto meta = std::make_unique<ArchiveMetaImpl>();
    meta->filter.reset(new Xp3Filter(input_file.path));
    p->filter_registry.set_decoder(*meta->filter);

    while (table_stream.tell() < table_stream.size())
        meta->entries.push_back(read_entry(logger, table_stream));
    return std::move(meta);
}

std::unique_ptr<io::File> Xp3ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    const auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    const auto entry = static_cast<const ArchiveEntryImpl*>(&e);

    bstr data;
    for (const auto &segm_chunk : entry->segm_chunks)
    {
        const auto data_is_compressed = segm_chunk->flags & 7;
        input_file.stream.seek(segm_chunk->offset);
        data += data_is_compressed
            ? algo::pack::zlib_inflate(
                input_file.stream.read(segm_chunk->size_comp))
            : input_file.stream.read(segm_chunk->size_orig);
    }

    if (meta->filter && meta->filter->decoder)
        meta->filter->decoder(data, entry->adlr_chunk->key);

    return std::make_unique<io::File>(entry->path, data);
}

void Xp3ArchiveDecoder::set_plugin(const std::string &plugin_name)
{
    p->filter_registry.use_plugin(plugin_name);
}

std::vector<std::string> Xp3ArchiveDecoder::get_linked_formats() const
{
    return {"kirikiri/tlg"};
}

static auto dummy = fmt::register_fmt<Xp3ArchiveDecoder>("kirikiri/xp3");
