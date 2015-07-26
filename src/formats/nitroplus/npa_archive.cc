// NPA archive
//
// Company:   Nitroplus
// Engine:    -
// Extension: .npa
//
// Known games:
// - Chaos;Head

#include "formats/nitroplus/npa_archive.h"
#include "formats/nitroplus/npa_filters/chaos_head.h"
#include "util/encoding.h"
#include "util/zlib.h"

using namespace au;
using namespace au::fmt::nitroplus;

namespace
{
    typedef enum
    {
        FILE_TYPE_DIRECTORY = 1,
        FILE_TYPE_FILE = 2
    } FileType;

    typedef struct
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
    } Header;

    typedef struct
    {
        std::string name;
        u32 offset;
        u32 size_compressed;
        u32 size_original;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;
}

static const std::string magic("NPA\x01\x00\x00\x00", 7);

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
    std::string &name,
    const Header &header,
    const NpaFilter &filter,
    size_t file_pos)
{
    u32 tmp = filter.file_name_key(header.key1, header.key2);
    for (size_t char_pos = 0; char_pos < name.size(); char_pos++)
    {
        u32 key = 0xfc * char_pos;
        key -= tmp >> 0x18;
        key -= tmp >> 0x10;
        key -= tmp >> 0x08;
        key -= tmp & 0xff;
        key -= file_pos >> 0x18;
        key -= file_pos >> 0x10;
        key -= file_pos >> 0x08;
        key -= file_pos;
        name[char_pos] += (key & 0xff);
    }
}

static void decrypt_file_data(
    std::string &data,
    const TableEntry &entry,
    const Header &header,
    const NpaFilter &filter)
{
    u32 key = filter.data_key;
    for (size_t i = 0; i < entry.name.size(); i++)
        key -= reinterpret_cast<const u8&>(entry.name[i]);
    key *= entry.name.size();
    key += header.key1 * header.key2;
    key *= entry.size_original;
    key &= 0xff;

    size_t length = 0x1000 + entry.name.size();
    for (size_t i = 0; i < length && i < entry.size_compressed; i++)
    {
        char p = filter.permutation[static_cast<u8>(data[i])];
        data[i] = static_cast<char>(p - key - i);
    }
}

static std::unique_ptr<TableEntry> read_table_entry(
    io::IO &arc_io,
    const Header &header,
    const NpaFilter &filter,
    size_t file_pos)
{
    std::unique_ptr<TableEntry> entry(new TableEntry);

    entry->name = arc_io.read(arc_io.read_u32_le());
    decrypt_file_name(entry->name, header, filter, file_pos);

    FileType file_type = static_cast<FileType>(arc_io.read_u8());
    arc_io.skip(4);

    entry->offset
        = arc_io.read_u32_le() + header.table_offset + header.table_size;
    entry->size_compressed = arc_io.read_u32_le();
    entry->size_original = arc_io.read_u32_le();

    if (file_type == FILE_TYPE_DIRECTORY)
        return nullptr;
    if (file_type != FILE_TYPE_FILE)
        throw std::runtime_error("Unknown file type");

    return entry;
}

static Table read_table(
    io::IO &arc_io, const Header &header, const NpaFilter &filter)
{
    Table table;
    for (size_t i = 0; i < header.total_count; i++)
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
    file->name = util::sjis_to_utf8(entry.name);

    arc_io.seek(entry.offset);
    std::string data = arc_io.read(entry.size_compressed);

    if (header.encrypted)
        decrypt_file_data(data, entry, header, filter);

    if (header.compressed)
        data = util::zlib_inflate(data);

    file->io.write(data);
    return file;
}

struct NpaArchive::Priv
{
    std::unique_ptr<NpaFilter> filter;
};

NpaArchive::NpaArchive() : p(new Priv)
{
}

NpaArchive::~NpaArchive()
{
}

void NpaArchive::add_cli_help(ArgParser &arg_parser) const
{
    arg_parser.add_help(
        "--plugin=PLUGIN",
        "Selects NPA decryption routine.\n"
            "Possible values:\n"
            "- chaos_head");

    Archive::add_cli_help(arg_parser);
}

void NpaArchive::parse_cli_options(const ArgParser &arg_parser)
{
    const std::string plugin = arg_parser.get_switch("plugin").c_str();
    void (*initializer)(NpaFilter&) = nullptr;
    if (plugin == "chaos_head")
        initializer = &npa_filters::chaos_head_filter_init;
    else
        throw std::runtime_error("Unrecognized plugin: " + plugin);

    if (initializer != nullptr)
    {
        p->filter.reset(new NpaFilter);
        initializer(*p->filter);
    }
    else
    {
        p->filter.reset(nullptr);
    }

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
        throw std::runtime_error("No plugin selected");

    std::unique_ptr<Header> header = read_header(arc_file.io);

    Table table = read_table(arc_file.io, *header, *p->filter);
    for (size_t i = 0; i < table.size(); i++)
    {
        file_saver.save(read_file(
            arc_file.io, *header, *p->filter, *table[i]));
    }
}
