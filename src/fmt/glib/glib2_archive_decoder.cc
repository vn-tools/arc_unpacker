#include "fmt/glib/glib2_archive_decoder.h"
#include "algo/range.h"
#include "err.h"
#include "fmt/glib/glib2/mei.h"
#include "fmt/glib/glib2/musume.h"
#include "io/memory_stream.h"

using namespace au;
using namespace au::fmt::glib;

namespace
{
    struct Header final
    {
        std::array<u32, 4> table_keys[4];
        bstr magic;
        size_t table_offset;
        size_t table_size;
    };

    struct ArchiveMetaImpl final : fmt::ArchiveMeta
    {
        std::shared_ptr<glib2::IPlugin> plugin;
    };

    struct ArchiveEntryImpl final : fmt::ArchiveEntry
    {
        std::array<u32, 4> content_keys[4];
        bool is_file;
        size_t offset;
        size_t size;
    };
}

static const bstr table_magic = "CDBD"_b;
static const bstr magic_21 = "GLibArchiveData2.1\x00"_b;
static const bstr magic_20 = "GLibArchiveData2.0\x00"_b;
static const size_t header_size = 0x5C;

static bstr decode(const bstr &input, const glib2::Decoder &decoder)
{
    bstr output(input.size());
    u32 acc = 0;
    while (acc < (input.size() & (~3)))
    {
        auto src_index = (acc & (~3)) + decoder.src_permutation[acc & 3];
        auto dst_index = (acc & (~3)) + decoder.dst_permutation[acc & 3];
        auto byte = input[src_index];
        byte = decoder.func2(decoder.func1(byte, acc), acc);
        output[dst_index] = byte;
        acc++;
    }
    while (acc < input.size())
    {
        output[acc] = decoder.func2(decoder.func1(input[acc], acc), acc);
        acc++;
    }
    return output;
}

static Header read_header(io::Stream &arc_stream, glib2::IPlugin &plugin)
{
    arc_stream.seek(0);
    auto decoder = plugin.create_header_decoder();
    auto buffer = decode(arc_stream.read(header_size), *decoder);
    io::MemoryStream header_stream(buffer);

    Header header;
    header.magic = header_stream.read(magic_21.size());
    header_stream.skip(1);
    for (auto i : algo::range(4))
    for (auto j : algo::range(4))
        header.table_keys[3 - i][j] = header_stream.read_u32_le();
    header.table_offset = header_stream.read_u32_le();
    header.table_size = header_stream.read_u32_le();
    return header;
}

static std::unique_ptr<ArchiveEntryImpl> read_table_entry(
    io::Stream &table_stream,
    const std::vector<std::unique_ptr<ArchiveEntryImpl>> &entries,
    size_t file_names_start,
    size_t file_headers_start)
{
    auto entry = std::make_unique<ArchiveEntryImpl>();

    auto file_name_offset = table_stream.read_u32_le() + file_names_start;
    table_stream.skip(4);
    s32 parent_dir = table_stream.read_u32_le();
    u32 flags = table_stream.read_u32_le();
    entry->is_file = flags == 0x100;
    auto file_header_offset = table_stream.read_u32_le() + file_headers_start;
    auto file_header_size = table_stream.read_u32_le();

    if (entry->is_file)
    {
        table_stream.peek(
            file_header_offset,
            [&]()
            {
                auto file_header_size2 = table_stream.read_u32_le();
                if (file_header_size2 + 4 != file_header_size)
                    throw err::BadDataSizeError();
                table_stream.skip(4); // null
                entry->size = table_stream.read_u32_le();
                entry->offset = table_stream.read_u32_le();
                for (auto i : algo::range(4))
                for (auto j : algo::range(4))
                    entry->content_keys[i][j] = table_stream.read_u32_le();
            });
    }

    table_stream.peek(
        file_name_offset,
        [&]()
        {
            std::string name = table_stream.read_to_zero().str();
            entry->path = parent_dir != -1
                ? entries.at(parent_dir)->path / name
                : name;
        });

    return entry;
}

static std::shared_ptr<glib2::IPlugin> guess_plugin(io::Stream &arc_stream)
{
    std::vector<std::shared_ptr<glib2::IPlugin>> plugins;
    plugins.push_back(std::make_shared<glib2::MeiPlugin>());
    plugins.push_back(std::make_shared<glib2::MusumePlugin>());

    for (auto &plugin : plugins)
    {
        try
        {
            auto header = read_header(arc_stream, *plugin);
            if (header.magic == magic_21 || header.magic == magic_21)
            {
                arc_stream.seek(0);
                return plugin;
            }
        }
        catch (...)
        {
            continue;
        }
    }
    return nullptr;
}

bool Glib2ArchiveDecoder::is_recognized_impl(io::File &input_file) const
{
    return guess_plugin(input_file.stream) != nullptr;
}

std::unique_ptr<fmt::ArchiveMeta> Glib2ArchiveDecoder::read_meta_impl(
    const Logger &logger, io::File &input_file) const
{
    auto plugin = guess_plugin(input_file.stream);
    auto header = read_header(input_file.stream, *plugin);

    input_file.stream.seek(header.table_offset);
    auto table_data = input_file.stream.read(header.table_size);
    for (const auto &key : header.table_keys)
        table_data = decode(table_data, *plugin->create_decoder(key));

    io::MemoryStream table_stream(table_data);
    if (table_stream.read(table_magic.size()) != table_magic)
        throw err::CorruptDataError("Corrupt table data");

    size_t file_count = table_stream.read_u32_le();
    size_t file_names_start = file_count * 0x18 + 0x10;
    size_t file_headers_start = table_stream.read_u32_le() + 0x10;
    size_t file_headers_size = table_stream.read_u32_le();

    std::vector<std::unique_ptr<ArchiveEntryImpl>> entries;
    for (auto i : algo::range(file_count))
        entries.push_back(read_table_entry(
            table_stream, entries, file_names_start, file_headers_start));

    auto meta = std::make_unique<ArchiveMetaImpl>();
    meta->plugin = plugin;
    for (auto &entry : entries)
        if (entry->is_file)
            meta->entries.push_back(std::move(entry));
    return std::move(meta);
}

std::unique_ptr<io::File> Glib2ArchiveDecoder::read_file_impl(
    const Logger &logger,
    io::File &input_file,
    const fmt::ArchiveMeta &m,
    const fmt::ArchiveEntry &e) const
{
    auto meta = static_cast<const ArchiveMetaImpl*>(&m);
    auto entry = static_cast<const ArchiveEntryImpl*>(&e);
    auto output_file = std::make_unique<io::File>();
    output_file->path = entry->path;

    input_file.stream.seek(entry->offset);

    const size_t chunk_size = 0x20000;
    size_t offset = 0;
    std::vector<std::unique_ptr<glib2::Decoder>> decoders(4);
    for (auto i : algo::range(4))
    {
        try
        {
            decoders[i] = meta->plugin->create_decoder(entry->content_keys[i]);
            offset += chunk_size;
            if (offset >= entry->size)
                break;
        }
        catch (err::NotSupportedError &e)
        {
            for (auto j : algo::range(i))
                decoders[i] = nullptr;
            logger.warn("%s\n", e.what());
        }
    }

    auto key_id = 0;
    for (auto written : algo::range(0, entry->size, chunk_size))
    {
        auto current_chunk_size = std::min<size_t>(
            chunk_size, entry->size - written);
        auto buffer = input_file.stream.read(current_chunk_size);
        if (decoders[key_id])
            buffer = decode(buffer, *decoders[key_id]);
        output_file->stream.write(buffer);
        key_id++;
        key_id %= 4;
    }

    return output_file;
}

std::vector<std::string> Glib2ArchiveDecoder::get_linked_formats() const
{
    return {"glib/pgx", "glib/jpeg-pgx", "vorbis/wav", "glib/glib2"};
}

static auto dummy = fmt::register_fmt<Glib2ArchiveDecoder>("glib/glib2");
