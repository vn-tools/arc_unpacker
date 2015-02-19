// NPA archive
//
// Company:   Nitroplus
// Engine:    -
// Extension: .npa
//
// Known games:
// - Chaos;Head

#include "formats/arc/npa_archive.h"
#include "formats/arc/npa_archive/npa_filter.h"
#include "formats/arc/npa_archive/npa_filter_chaos_head.h"
#include "string/encoding.h"
#include "string/zlib.h"

namespace
{
    const std::string magic("NPA\x01\x00\x00\x00", 7);

    typedef enum
    {
        FILE_TYPE_DIRECTORY = 1,
        FILE_TYPE_FILE = 2
    } FileType;

    typedef struct
    {
        uint32_t key1;
        uint32_t key2;
        bool compressed;
        bool encrypted;
        uint32_t total_count;
        uint32_t folder_count;
        uint32_t file_count;
        uint32_t table_size;
        size_t table_offset;
    } Header;

    typedef struct
    {
        std::string name;
        uint32_t offset;
        uint32_t size_compressed;
        uint32_t size_original;
    } TableEntry;

    typedef std::vector<std::unique_ptr<TableEntry>> Table;

    std::unique_ptr<Header> read_header(IO &arc_io)
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

    void decrypt_file_name(
        std::string &name,
        const Header &header,
        const NpaFilter &filter,
        size_t file_pos)
    {
        uint32_t tmp = filter.file_name_key(header.key1, header.key2);
        for (size_t char_pos = 0; char_pos < name.size(); char_pos++)
        {
            uint32_t key = 0xfc * char_pos;
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

    void decrypt_file_data(
        std::string &data,
        const TableEntry &table_entry,
        const Header &header,
        const NpaFilter &filter)
    {
        uint32_t key = filter.data_key;
        for (size_t i = 0; i < table_entry.name.size(); i ++)
            key -= reinterpret_cast<const unsigned char&>(table_entry.name[i]);
        key *= table_entry.name.size();
        key += header.key1 * header.key2;
        key *= table_entry.size_original;
        key &= 0xff;

        size_t length = 0x1000 + table_entry.name.size();
        for (size_t i = 0; i < length && i < table_entry.size_compressed; i ++)
        {
            char p = filter.permutation[static_cast<unsigned char>(data[i])];
            data[i] = static_cast<char>(p - key - i);
        }
    }

    std::unique_ptr<TableEntry> read_table_entry(
        IO &arc_io,
        const Header &header,
        const NpaFilter &filter,
        size_t file_pos)
    {
        std::unique_ptr<TableEntry> table_entry(new TableEntry);

        table_entry->name = arc_io.read(arc_io.read_u32_le());
        decrypt_file_name(table_entry->name, header, filter, file_pos);

        FileType file_type = static_cast<FileType>(arc_io.read_u8());
        arc_io.skip(4);

        table_entry->offset
            = arc_io.read_u32_le() + header.table_offset + header.table_size;
        table_entry->size_compressed = arc_io.read_u32_le();
        table_entry->size_original = arc_io.read_u32_le();

        if (file_type == FILE_TYPE_DIRECTORY)
            return nullptr;
        if (file_type != FILE_TYPE_FILE)
            throw std::runtime_error("Unknown file type");

        return table_entry;
    }

    Table read_table(
        IO &arc_io, const Header &header, const NpaFilter &filter)
    {
        Table table;
        for (size_t i = 0; i < header.total_count; i ++)
        {
            auto table_entry = read_table_entry(arc_io, header, filter, i);
            if (table_entry != nullptr)
                table.push_back(std::move(table_entry));
        }
        return table;
    }

    std::unique_ptr<File> read_file(
        IO &arc_io,
        const Header &header,
        const NpaFilter &filter,
        const TableEntry &table_entry)
    {
        std::unique_ptr<File> file(new File);
        file->name = convert_encoding(table_entry.name, "cp932", "utf-8");

        arc_io.seek(table_entry.offset);
        std::string data = arc_io.read(table_entry.size_compressed);

        if (header.encrypted)
            decrypt_file_data(data, table_entry, header, filter);

        if (header.compressed)
            data = zlib_inflate(data);

        file->io.write(data);
        return file;
    }
}

struct NpaArchive::Internals
{
    std::unique_ptr<NpaFilter> filter;
};

NpaArchive::NpaArchive() : internals(new Internals)
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
}

void NpaArchive::parse_cli_options(ArgParser &arg_parser)
{
    const std::string plugin = arg_parser.get_switch("plugin").c_str();
    void (*initializer)(NpaFilter&) = nullptr;
    if (plugin == "chaos_head")
        initializer = &npa_chaos_head_filter_init;
    else
        throw std::runtime_error("Unrecognized plugin: " + plugin);

    if (initializer != nullptr)
    {
        internals->filter.reset(new NpaFilter);
        initializer(*internals->filter);
    }
    else
    {
        internals->filter.reset(nullptr);
    }
}

void NpaArchive::unpack_internal(File &file, FileSaver &file_saver) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a NPA archive");

    if (internals->filter == nullptr)
        throw std::runtime_error("No plugin selected");

    std::unique_ptr<Header> header = read_header(file.io);

    Table table = read_table(file.io, *header, *internals->filter);
    for (size_t i = 0; i < table.size(); i ++)
    {
        file_saver.save(read_file(
            file.io, *header, *internals->filter, *table[i]));
    }
}
