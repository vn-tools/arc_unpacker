// NPA archive
//
// Company:   Nitroplus
// Engine:    -
// Extension: .npa
//
// Known games:
// - [Nitroplus] [080425] CHAOS;HEAD

#include "fmt/nitroplus/npa_archive.h"
#include "fmt/nitroplus/npa_filters/chaos_head.h"
#include "util/encoding.h"
#include "util/pack/zlib.h"
#include "util/plugin_mgr.hh"
#include "util/range.h"

using namespace au;
using namespace au::fmt::nitroplus;

namespace
{
    enum FileType
    {
        FILE_TYPE_DIRECTORY = 1,
        FILE_TYPE_FILE = 2
    };

    struct Header
    {
        u32 key1;
        u32 key2;
        bool compressed;
        bool encrypted;
        u32 total_count;
        u32 folder_count;
        u32 file_count;
        u32 table_size;
        size_t table_offset;
    };

    struct TableEntry
    {
        bstr name_original;
        std::string name;
        u32 offset;
        u32 size_compressed;
        u32 size_original;
    };

    using Table = std::vector<std::unique_ptr<TableEntry>>;
}

static const bstr magic = "NPA\x01\x00\x00\x00"_b;

static std::unique_ptr<Header> read_header(io::IO &arc_io)
{
    std::unique_ptr<Header> header(new Header);
    header->key1 = arc_io.read_u32_le();
    header->key2 = arc_io.read_u32_le();
    header->compressed = arc_io.read_u8() > 0;
    header->encrypted = arc_io.read_u8() > 0;
    header->total_count = arc_io.read_u32_le();
    header->folder_count = arc_io.read_u32_le();
    header->file_count = arc_io.read_u32_le();
    arc_io.skip(8);
    header->table_size = arc_io.read_u32_le();
    header->table_offset = arc_io.tell();
    return header;
}

static void decrypt_file_name(
    bstr &name,
    const Header &header,
    const NpaFilter &filter,
    size_t file_pos)
{
    u32 tmp = filter.file_name_key(header.key1, header.key2);
    for (auto char_pos : util::range(name.size()))
    {
        u32 key = 0xFC * char_pos;
        key -= tmp >> 0x18;
        key -= tmp >> 0x10;
        key -= tmp >> 0x08;
        key -= tmp & 0xFF;
        key -= file_pos >> 0x18;
        key -= file_pos >> 0x10;
        key -= file_pos >> 0x08;
        key -= file_pos;
        name.get<u8>()[char_pos] += (key & 0xFF);
    }
}

static void decrypt_file_data(
    bstr &data,
    const TableEntry &entry,
    const Header &header,
    const NpaFilter &filter)
{
    u32 key = filter.data_key;
    for (auto i : util::range(entry.name_original.size()))
        key -= entry.name_original.get<u8>()[i];
    key *= entry.name_original.size();
    key += header.key1 * header.key2;
    key *= entry.size_original;
    key &= 0xFF;

    u8 *data_ptr = data.get<u8>();
    size_t size = 0x1000 + entry.name_original.size();
    for (auto i : util::range(std::min(size, data.size())))
        data_ptr[i] = filter.permutation[data_ptr[i]] - key - i;
}

static std::unique_ptr<TableEntry> read_table_entry(
    io::IO &arc_io,
    const Header &header,
    const NpaFilter &filter,
    size_t file_pos)
{
    std::unique_ptr<TableEntry> entry(new TableEntry);

    entry->name_original = arc_io.read(arc_io.read_u32_le());
    decrypt_file_name(entry->name_original, header, filter, file_pos);
    entry->name = util::sjis_to_utf8(entry->name_original).str();

    FileType file_type = static_cast<FileType>(arc_io.read_u8());
    arc_io.skip(4);

    entry->offset
        = arc_io.read_u32_le() + header.table_offset + header.table_size;
    entry->size_compressed = arc_io.read_u32_le();
    entry->size_original = arc_io.read_u32_le();

    if (file_type == FILE_TYPE_DIRECTORY)
        return nullptr;
    if (file_type != FILE_TYPE_FILE)
        throw err::NotSupportedError("Unknown file type");

    return entry;
}

static Table read_table(
    io::IO &arc_io, const Header &header, const NpaFilter &filter)
{
    Table table;
    for (auto i : util::range(header.total_count))
    {
        auto entry = read_table_entry(arc_io, header, filter, i);
        if (entry != nullptr)
            table.push_back(std::move(entry));
    }
    return table;
}

static std::unique_ptr<File> read_file(
    io::IO &arc_io,
    const Header &header,
    const NpaFilter &filter,
    const TableEntry &entry)
{
    std::unique_ptr<File> file(new File);
    file->name = entry.name;

    arc_io.seek(entry.offset);
    auto data = arc_io.read(entry.size_compressed);

    if (header.encrypted)
        decrypt_file_data(data, entry, header, filter);

    if (header.compressed)
        data = util::pack::zlib_inflate(data);

    file->io.write(data);
    return file;
}

struct NpaArchive::Priv
{
    util::PluginManager<std::function<std::unique_ptr<NpaFilter>()>> plugin_mgr;
    std::unique_ptr<NpaFilter> filter;
};

NpaArchive::NpaArchive() : p(new Priv)
{
    p->plugin_mgr.add("chaos_head", "ChaoS;HEAd", []()
        {
            std::unique_ptr<NpaFilter> filter(new NpaFilter);
            npa_filters::chaos_head_filter_init(*filter);
            return filter;
        });
}

NpaArchive::~NpaArchive()
{
}

void NpaArchive::register_cli_options(ArgParser &arg_parser) const
{
    p->plugin_mgr.register_cli_options(
        arg_parser, "Selects NPA decryption routine.");
    Archive::register_cli_options(arg_parser);
}

void NpaArchive::parse_cli_options(const ArgParser &arg_parser)
{
    p->filter = p->plugin_mgr.get_from_cli_options(arg_parser, true)();
    Archive::parse_cli_options(arg_parser);
}

bool NpaArchive::is_recognized_internal(File &arc_file) const
{
    return arc_file.io.read(magic.size()) == magic;
}

void NpaArchive::unpack_internal(File &arc_file, FileSaver &file_saver) const
{
    arc_file.io.skip(magic.size());

    if (p->filter == nullptr)
        throw err::UsageError("No plugin selected");

    std::unique_ptr<Header> header = read_header(arc_file.io);
    Table table = read_table(arc_file.io, *header, *p->filter);
    for (auto &entry : table)
        file_saver.save(read_file(arc_file.io, *header, *p->filter, *entry));
}

static auto dummy = fmt::Registry::add<NpaArchive>("nitro/npa");
