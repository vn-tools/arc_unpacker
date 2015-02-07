// NPA archive
//
// Company:   Nitroplus
// Engine:    -
// Extension: .npa
//
// Known games:
// - Chaos;Head

#include "string_ex.h"
#include "formats/arc/npa_archive.h"
#include "formats/arc/npa_archive/npa_filter.h"
#include "formats/arc/npa_archive/npa_filter_chaos_head.h"

namespace
{
    const std::string npa_magic("NPA\x01\x00\x00\x00", 7);

    typedef enum
    {
        NPA_FILE_TYPE_DIRECTORY = 1,
        NPA_FILE_TYPE_FILE = 2
    } NpaFileType;

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
    } NpaHeader;

    typedef struct NpaUnpackContext
    {
        IO &arc_io;
        NpaHeader &header;
        NpaFilter *filter;
        size_t file_pos;
        size_t table_offset;

        NpaUnpackContext(IO &arc_io, NpaHeader header)
            : arc_io(arc_io), header(header)
        {
        }
    } NpaUnpackContext;

    std::unique_ptr<NpaHeader> npa_read_header(IO &arc_io)
    {
        std::unique_ptr<NpaHeader> header(new NpaHeader);
        header->key1 = arc_io.read_u32_le();
        header->key2 = arc_io.read_u32_le();
        header->compressed = arc_io.read_u8();
        header->encrypted = arc_io.read_u8();
        header->total_count = arc_io.read_u32_le();
        header->folder_count = arc_io.read_u32_le();
        header->file_count = arc_io.read_u32_le();
        arc_io.skip(8);
        header->table_size = arc_io.read_u32_le();
        return header;
    }

    void npa_decrypt_file_name(
        std::string &name, NpaUnpackContext *unpack_context)
    {
        uint32_t tmp = unpack_context->filter->file_name_key(
            unpack_context->header.key1,
            unpack_context->header.key2);

        size_t char_pos;
        for (char_pos = 0; char_pos < name.size(); char_pos++)
        {
            uint32_t key = 0xfc * char_pos;
            key -= tmp >> 0x18;
            key -= tmp >> 0x10;
            key -= tmp >> 0x08;
            key -= tmp & 0xff;
            key -= unpack_context->file_pos >> 0x18;
            key -= unpack_context->file_pos >> 0x10;
            key -= unpack_context->file_pos >> 0x08;
            key -= unpack_context->file_pos;
            name[char_pos] += (key & 0xff);
        }
    }

    std::string npa_read_file_name(NpaUnpackContext *unpack_context)
    {
        size_t file_name_length = unpack_context->arc_io.read_u32_le();
        std::string name = unpack_context->arc_io.read(file_name_length);
        npa_decrypt_file_name(name, unpack_context);
        return name;
    }

    void npa_decrypt_file_data(
        unsigned char *data,
        size_t data_size_compressed,
        size_t data_size_original,
        const std::string &original_file_name,
        NpaUnpackContext *unpack_context)
    {
        const unsigned char *permutation = unpack_context->filter->permutation;

        uint32_t key = unpack_context->filter->data_key;
        size_t i;
        for (i = 0; i < original_file_name.size(); i ++)
            key -= (unsigned char)original_file_name[i];
        key *= original_file_name.size();
        key += unpack_context->header.key1 * unpack_context->header.key2;
        key *= data_size_original;
        key &= 0xff;

        size_t length = 0x1000 + original_file_name.size();
        for (i = 0; i < length && i < data_size_compressed; i ++)
            data[i] = (unsigned char)(permutation[(size_t)data[i]] - key - i);
    }

    std::string npa_read_file_data(
        size_t size_compressed,
        size_t size_original,
        const std::string &original_file_name,
        NpaUnpackContext *unpack_context)
    {
        std::string data = unpack_context->arc_io.read(size_compressed);

        if (unpack_context->header.encrypted)
        {
            npa_decrypt_file_data(
                (unsigned char*)data.data(),
                size_compressed,
                size_original,
                original_file_name,
                unpack_context);
        }

        if (unpack_context->header.compressed)
            data = zlib_inflate(data);

        return data;
    }

    std::unique_ptr<VirtualFile> npa_read_file(void *_context)
    {
        NpaUnpackContext *unpack_context = (NpaUnpackContext*)_context;
        std::unique_ptr<VirtualFile> file(new VirtualFile);

        std::string file_name = npa_read_file_name(unpack_context);
        file->name = convert_encoding(file_name, "cp932", "utf-8");

        NpaFileType file_type = (NpaFileType)unpack_context->arc_io.read_u8();
        unpack_context->arc_io.skip(4);
        uint32_t offset = unpack_context->arc_io.read_u32_le();
        uint32_t size_compressed = unpack_context->arc_io.read_u32_le();
        uint32_t size_original = unpack_context->arc_io.read_u32_le();
        offset +=
            unpack_context->table_offset + unpack_context->header.table_size;

        if (file_type == NPA_FILE_TYPE_DIRECTORY)
            return nullptr;

        else if (file_type != NPA_FILE_TYPE_FILE)
            throw std::runtime_error("Unknown file type");

        size_t old_pos = unpack_context->arc_io.tell();
        unpack_context->arc_io.seek(offset);
        std::string file_data = npa_read_file_data(
            size_compressed, size_original, file_name, unpack_context);
        file->io.write(file_data);
        unpack_context->arc_io.seek(old_pos);

        return file;
    }
}

struct NpaArchive::Context
{
    NpaFilter *filter;
};

NpaArchive::NpaArchive()
{
    context = new NpaArchive::Context();
}

NpaArchive::~NpaArchive()
{
    delete context;
}

void NpaArchive::add_cli_help(ArgParser &arg_parser)
{
    arg_parser.add_help(
        "--plugin=PLUGIN",
        "Selects NPA decryption routine.\n"
            "Possible values:\n"
            "- chaos_head");
}

void NpaArchive::parse_cli_options(ArgParser &arg_parser)
{
    context->filter = new NpaFilter;

    const std::string plugin = arg_parser.get_switch("plugin").c_str();
    void (*initializer)(NpaFilter*) = nullptr;
    if (plugin == "chaos_head")
        initializer = &npa_chaos_head_filter_init;
    else
        throw std::runtime_error("Unrecognized plugin: " + plugin);

    if (initializer != nullptr)
        initializer(context->filter);
    else
    {
        delete context->filter;
        context->filter = nullptr;
    }
}

void NpaArchive::unpack_internal(IO &arc_io, OutputFiles &output_files) const
{
    if (arc_io.read(npa_magic.size()) != npa_magic)
        throw std::runtime_error("Not a NPA archive");

    if (context->filter == nullptr)
        throw std::runtime_error("No plugin selected");

    std::unique_ptr<NpaHeader> header = npa_read_header(arc_io);
    NpaUnpackContext unpack_context(arc_io, *header);
    unpack_context.filter = context->filter;
    unpack_context.table_offset = arc_io.tell();
    size_t file_pos;
    for (file_pos = 0; file_pos < header->total_count; file_pos ++)
    {
        unpack_context.file_pos = file_pos;
        output_files.save(&npa_read_file, &unpack_context);
    }
}
