// NPA archive
//
// Company:   Nitroplus
// Engine:    -
// Extension: .npa
//
// Known games:
// - Chaos;Head

#include <cassert>
#include <cstring>
#include "string_ex.h"
#include "formats/arc/npa_archive.h"
#include "formats/arc/npa_archive/npa_filter.h"
#include "formats/arc/npa_archive/npa_filter_chaos_head.h"
#include "logger.h"

namespace
{
    const char *npa_magic = "NPA\x01\x00\x00\x00";
    const size_t npa_magic_length = 7;

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
        NpaFilter *filter;
        NpaHeader *header;
        size_t file_pos;
        size_t table_offset;

        NpaUnpackContext(IO &arc_io) : arc_io(arc_io)
        {
        }
    } NpaUnpackContext;

    bool npa_check_magic(IO &arc_io)
    {
        char magic[npa_magic_length];
        arc_io.read(magic, npa_magic_length);
        return memcmp(magic, npa_magic, npa_magic_length) == 0;
    }

    NpaHeader *npa_read_header(IO &arc_io)
    {
        NpaHeader *header = new NpaHeader;
        assert(header != nullptr);
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
        char *name, size_t name_length, NpaUnpackContext *unpack_context)
    {
        assert(name != nullptr);
        assert(unpack_context != nullptr);

        uint32_t tmp = unpack_context->filter->file_name_key(
            unpack_context->header->key1,
            unpack_context->header->key2);

        size_t char_pos;
        for (char_pos = 0; char_pos < name_length; char_pos++)
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

    char *npa_read_file_name(
        NpaUnpackContext *unpack_context, size_t *file_name_length)
    {
        assert(unpack_context != nullptr);
        assert(file_name_length != nullptr);

        *file_name_length = unpack_context->arc_io.read_u32_le();
        char *file_name = new char[*file_name_length];
        assert(file_name != nullptr);
        unpack_context->arc_io.read(file_name, *file_name_length);
        npa_decrypt_file_name(file_name, *file_name_length, unpack_context);
        return file_name;
    }

    void npa_decrypt_file_data(
        unsigned char *data,
        size_t data_size_compressed,
        size_t data_size_original,
        const char *original_file_name,
        size_t original_file_name_length,
        NpaUnpackContext *unpack_context)
    {
        assert(data != nullptr);
        assert(unpack_context != nullptr);

        const unsigned char *permutation
            = unpack_context->filter->permutation;

        uint32_t key = unpack_context->filter->data_key;
        size_t i;
        for (i = 0; i < original_file_name_length; i ++)
            key -= (unsigned char)original_file_name[i];
        key *= original_file_name_length;
        key += unpack_context->header->key1 * unpack_context->header->key2;
        key *= data_size_original;
        key &= 0xff;

        size_t length = 0x1000 + original_file_name_length;
        for (i = 0; i < length && i < data_size_compressed; i ++)
            data[i] = (unsigned char)(permutation[(size_t)data[i]] - key - i);
    }

    char *npa_read_file_data(
        size_t size_compressed,
        size_t size_original,
        const char *original_file_name,
        size_t original_file_name_length,
        NpaUnpackContext *unpack_context)
    {
        assert(original_file_name != nullptr);
        assert(unpack_context != nullptr);

        char *data = new char[size_compressed];
        assert(data != nullptr);
        unpack_context->arc_io.read(data, size_compressed);

        if (unpack_context->header->encrypted)
        {
            npa_decrypt_file_data(
                (unsigned char*)data,
                size_compressed,
                size_original,
                original_file_name,
                original_file_name_length,
                unpack_context);
        }

        if (unpack_context->header->compressed)
        {
            char *data_uncompressed = nullptr;
            if (!zlib_inflate(
                data,
                size_compressed,
                &data_uncompressed,
                nullptr))
            {
                assert(0);
            }
            assert(data_uncompressed != nullptr);
            delete []data;
            data = data_uncompressed;
        }

        return data;
    }

    std::unique_ptr<VirtualFile> npa_read_file(void *_context)
    {
        NpaUnpackContext *unpack_context = (NpaUnpackContext*)_context;
        std::unique_ptr<VirtualFile> file(new VirtualFile);

        size_t file_name_length;
        char *file_name = npa_read_file_name(unpack_context, &file_name_length);
        char *file_name_utf8;
        convert_encoding(
            file_name, file_name_length,
            &file_name_utf8, nullptr,
            "cp932", "utf-8");
        assert(file_name_utf8 != nullptr);
        file->name = std::string(file_name_utf8);

        NpaFileType file_type = (NpaFileType)unpack_context->arc_io.read_u8();
        unpack_context->arc_io.skip(4);
        uint32_t offset = unpack_context->arc_io.read_u32_le();
        uint32_t size_compressed = unpack_context->arc_io.read_u32_le();
        uint32_t size_original = unpack_context->arc_io.read_u32_le();
        offset +=
            unpack_context->table_offset + unpack_context->header->table_size;

        if (file_type == NPA_FILE_TYPE_DIRECTORY)
        {
            log_info("NPA: Empty directory - ignoring.");
            return nullptr;
        }
        else if (file_type != NPA_FILE_TYPE_FILE)
        {
            log_error("NPA: Unknown file type: %d", file_type);
            return nullptr;
        }

        size_t old_pos = unpack_context->arc_io.tell();
        unpack_context->arc_io.seek(offset);
        char *file_data = npa_read_file_data(
            size_compressed,
            size_original,
            file_name,
            file_name_length,
            unpack_context);
        file->io.write(file_data, size_original);
        delete []file_data;
        delete []file_name;
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
    assert(context != nullptr);

    context->filter = new NpaFilter;
    assert(context->filter != nullptr);

    const char *plugin = arg_parser.get_switch("plugin").c_str();
    void (*initializer)(NpaFilter*) = nullptr;
    if (plugin != nullptr)
    {
        if (strcmp(plugin, "chaos_head") == 0)
        {
            initializer = &npa_chaos_head_filter_init;
        }
        else
        {
            log_error("NPA: Unrecognized plugin: %s", plugin);
        }
    }

    if (initializer != nullptr)
        initializer(context->filter);
    else
    {
        delete context->filter;
        context->filter = nullptr;
    }
}

bool NpaArchive::unpack_internal(IO &arc_io, OutputFiles &output_files)
{
    assert(context != nullptr);
    if (!npa_check_magic(arc_io))
    {
        log_error("NPA: Not a NPA archive");
        return false;
    }

    if (context->filter == nullptr)
    {
        log_error("NPA: No plugin selected");
        return false;
    }

    NpaHeader *header = npa_read_header(arc_io);
    NpaUnpackContext unpack_context(arc_io);
    unpack_context.header = header;
    unpack_context.filter = context->filter;
    unpack_context.table_offset = arc_io.tell();
    size_t file_pos;
    for (file_pos = 0; file_pos < header->total_count; file_pos ++)
    {
        unpack_context.file_pos = file_pos;
        output_files.save(&npa_read_file, &unpack_context);
    }
    delete header;
    return true;
}
