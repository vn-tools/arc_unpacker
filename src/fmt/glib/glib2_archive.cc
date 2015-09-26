#include "fmt/glib/glib2_archive.h"
#include "err.h"
#include "fmt/glib/glib2/mei.h"
#include "fmt/glib/glib2/musume.h"
#include "fmt/glib/jpeg_pgx_converter.h"
#include "fmt/glib/pgx_converter.h"
#include "fmt/vorbis/packed_ogg_converter.h"
#include "io/buffered_io.h"
#include "log.h"
#include "util/range.h"

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

    struct TableEntry final
    {
        std::array<u32, 4> content_keys[4];
        std::string name;
        bool is_file;
        u32 offset;
        u32 size;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
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

static std::unique_ptr<Header> read_header(
    io::IO &arc_io, glib2::Plugin &plugin)
{
    auto decoder = plugin.create_header_decoder();
    auto buffer = decode(arc_io.read(header_size), *decoder);
    io::BufferedIO header_io(buffer);

    std::unique_ptr<Header> header(new Header);
    header->magic = header_io.read(magic_21.size());
    header_io.skip(1);
    for (auto i : util::range(4))
    for (auto j : util::range(4))
        header->table_keys[3 - i][j] = header_io.read_u32_le();
    header->table_offset = header_io.read_u32_le();
    header->table_size = header_io.read_u32_le();
    return header;
}

static std::unique_ptr<TableEntry> read_table_entry(
    io::IO &table_io,
    Table &table,
    size_t file_names_start,
    size_t file_headers_start)
{
    std::unique_ptr<TableEntry> entry(new TableEntry());

    auto file_name_offset = table_io.read_u32_le() + file_names_start;
    table_io.skip(4);
    s32 parent_dir = table_io.read_u32_le();
    u32 flags = table_io.read_u32_le();
    entry->is_file = flags == 0x100;
    auto file_header_offset = table_io.read_u32_le() + file_headers_start;
    auto file_header_size = table_io.read_u32_le();

    if (entry->is_file)
    {
        table_io.peek(
            file_header_offset,
            [&]()
            {
                auto file_header_size2 = table_io.read_u32_le();
                if (file_header_size2 + 4 != file_header_size)
                    throw err::BadDataSizeError();
                table_io.skip(4); // null
                entry->size = table_io.read_u32_le();
                entry->offset = table_io.read_u32_le();
                for (auto i : util::range(4))
                for (auto j : util::range(4))
                    entry->content_keys[i][j] = table_io.read_u32_le();
            });
    }

    table_io.peek(
        file_name_offset,
        [&]()
        {
            std::string name = table_io.read_to_zero().str();
            entry->name = parent_dir != -1
                ? table[parent_dir]->name + "/" + name
                : name;
        });

    return entry;
}

static Table read_table(
    io::IO &arc_io, const Header &header, const glib2::Plugin &plugin)
{
    arc_io.seek(header.table_offset);
    auto buffer = arc_io.read(header.table_size);
    for (const auto &key : header.table_keys)
        buffer = decode(buffer, *plugin.create_decoder(key));

    io::BufferedIO table_io(buffer);
    if (table_io.read(table_magic.size()) != table_magic)
        throw err::CorruptDataError("Corrupt table data");

    size_t file_count = table_io.read_u32_le();
    size_t file_names_start = file_count * 0x18 + 0x10;
    size_t file_headers_start = table_io.read_u32_le() + 0x10;
    size_t file_headers_size = table_io.read_u32_le();

    Table table(file_count);
    for (auto i : util::range(file_count))
    {
        table[i] = read_table_entry(
            table_io, table, file_names_start, file_headers_start);
    }
    return table;
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io, const TableEntry &entry, const glib2::Plugin &plugin)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;

    arc_io.seek(entry.offset);

    const size_t chunk_size = 0x20000;
    size_t offset = 0;
    std::vector<std::unique_ptr<glib2::Decoder>> decoders(4);
    for (auto i : util::range(4))
    {
        try
        {
            decoders[i] = plugin.create_decoder(entry.content_keys[i]);
            offset += chunk_size;
            if (offset >= entry.size)
                break;
        }
        catch (err::NotSupportedError &e)
        {
            for (auto j : util::range(i))
                decoders[i] = nullptr;
            Log.warn(std::string(e.what()) + "\n");
        }
    }

    auto key_id = 0;
    for (auto written : util::range(0, entry.size, chunk_size))
    {
        auto current_chunk_size = std::min<size_t>(
            chunk_size, entry.size - written);
        auto buffer = arc_io.read(current_chunk_size);
        if (decoders[key_id])
            buffer = decode(buffer, *decoders[key_id]);
        file->io.write(buffer);
        key_id++;
        key_id %= 4;
    }

    return file;
}

static std::unique_ptr<glib2::Plugin> guess_plugin(io::IO &arc_io)
{
    std::vector<std::unique_ptr<glib2::Plugin>> plugins;
    plugins.push_back(std::unique_ptr<glib2::Plugin>(
        new glib2::MeiPlugin()));
    plugins.push_back(std::unique_ptr<glib2::Plugin>(
        new glib2::MusumePlugin()));

    for (auto &plugin : plugins)
    {
        try
        {
            arc_io.seek(0);
            auto header = read_header(arc_io, *plugin);
            if (header->magic == magic_21 || header->magic == magic_21)
            {
                arc_io.seek(0);
                return std::move(plugin);
            }
        }
        catch (...)
        {
            continue;
        }
    }
    return nullptr;
}

struct Glib2Archive::Priv final
{
    PgxConverter pgx_converter;
    JpegPgxConverter jpeg_pgx_converter;
    fmt::vorbis::PackedOggConverter packed_ogg_converter;
};

Glib2Archive::Glib2Archive() : p(new Priv)
{
    add_decoder(&p->pgx_converter);
    add_decoder(&p->jpeg_pgx_converter);
    add_decoder(&p->packed_ogg_converter);
    add_decoder(this);
}

Glib2Archive::~Glib2Archive()
{
}

bool Glib2Archive::is_recognized_internal(File &arc_file) const
{
    return guess_plugin(arc_file.io) != nullptr;
}

void Glib2Archive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    auto plugin = guess_plugin(arc_file.io);
    auto header = read_header(arc_file.io, *plugin);
    auto table = read_table(arc_file.io, *header, *plugin);
    for (auto &entry : table)
        if (entry->is_file)
            file_saver.save(read_file(arc_file.io, *entry, *plugin));
}

static auto dummy = fmt::Registry::add<Glib2Archive>("glib/g2");
