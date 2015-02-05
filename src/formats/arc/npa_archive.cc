// NPA archive
//
// Company:   Nitroplus
// Engine:    -
// Extension: .npa
//
// Known games:
// - Chaos;Head

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "string_ex.h"
#include "formats/arc/npa_archive.h"
#include "formats/arc/npa_archive/npa_filter.h"
#include "formats/arc/npa_archive/npa_filter_chaos_head.h"
#include "logger.h"

static const char *npa_magic = "NPA\x01\x00\x00\x00";
static const size_t npa_magic_length = 7;

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

typedef struct
{
    NpaFilter *filter;
} NpaArchiveContext;

typedef struct
{
    IO *arc_io;
    NpaArchiveContext *archive_context;
    NpaHeader *header;
    size_t file_pos;
    size_t table_offset;
} NpaUnpackContext;



static void npa_add_cli_help(
    __attribute__((unused)) Archive *archive,
    ArgParser *arg_parser)
{
    arg_parser_add_help(
        arg_parser,
        "--plugin=PLUGIN",
        "Selects NPA decryption routine.\n"
            "Possible values:\n"
            "- chaos_head");
}

static void npa_parse_cli_options(Archive *archive, ArgParser *arg_parser)
{
    NpaArchiveContext *archive_context
        = (NpaArchiveContext*)malloc(sizeof(NpaArchiveContext));
    assert(archive_context != NULL);

    archive_context->filter = (NpaFilter*)malloc(sizeof(NpaFilter));
    assert(archive_context->filter != NULL);

    const char *plugin = arg_parser_get_switch(arg_parser, "plugin");
    void (*initializer)(NpaFilter*) = NULL;
    if (plugin != NULL)
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

    if (initializer != NULL)
        initializer(archive_context->filter);
    else
    {
        free(archive_context->filter);
        archive_context->filter = NULL;
    }

    archive->data = archive_context;
}

static void npa_cleanup(Archive *archive)
{
    NpaArchiveContext *archive_context = (NpaArchiveContext*)archive->data;
    free(archive_context);
}



static bool npa_check_magic(IO *arc_io)
{
    char magic[npa_magic_length];
    io_read_string(arc_io, magic, npa_magic_length);
    return memcmp(magic, npa_magic, npa_magic_length) == 0;
}

static NpaHeader *npa_read_header(IO *arc_io)
{
    NpaHeader *header = (NpaHeader*)malloc(sizeof(NpaHeader));
    assert(header != NULL);
    header->key1 = io_read_u32_le(arc_io);
    header->key2 = io_read_u32_le(arc_io);
    header->compressed = io_read_u8(arc_io);
    header->encrypted = io_read_u8(arc_io);
    header->total_count = io_read_u32_le(arc_io);
    header->folder_count = io_read_u32_le(arc_io);
    header->file_count = io_read_u32_le(arc_io);
    io_skip(arc_io, 8);
    header->table_size = io_read_u32_le(arc_io);
    return header;
}

static void npa_decrypt_file_name(
    char *name,
    size_t name_length,
    NpaUnpackContext *unpack_context)
{
    assert(name != NULL);
    assert(unpack_context != NULL);

    uint32_t tmp = unpack_context->archive_context->filter->file_name_key(
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

static char *npa_read_file_name(
    NpaUnpackContext *unpack_context,
    size_t *file_name_length)
{
    assert(unpack_context != NULL);
    assert(file_name_length != NULL);

    *file_name_length = io_read_u32_le(unpack_context->arc_io);
    char *file_name = (char*)malloc(*file_name_length);
    assert(file_name != NULL);
    io_read_string(unpack_context->arc_io, file_name, *file_name_length);
    npa_decrypt_file_name(file_name, *file_name_length, unpack_context);
    return file_name;
}

static void npa_decrypt_file_data(
    unsigned char *data,
    size_t data_size_compressed,
    size_t data_size_original,
    const char *original_file_name,
    size_t original_file_name_length,
    NpaUnpackContext *unpack_context)
{
    assert(data != NULL);
    assert(unpack_context != NULL);

    const unsigned char *permutation
        = unpack_context->archive_context->filter->permutation;

    uint32_t key = unpack_context->archive_context->filter->data_key;
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

static char *npa_read_file_data(
    size_t size_compressed,
    size_t size_original,
    const char *original_file_name,
    size_t original_file_name_length,
    NpaUnpackContext *unpack_context)
{
    assert(original_file_name != NULL);
    assert(unpack_context != NULL);

    char *data = (char*)malloc(size_compressed);
    assert(data != NULL);
    if (!io_read_string(unpack_context->arc_io, data, size_compressed))
        assert(0);

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
        char *data_uncompressed = NULL;
        if (!zlib_inflate(
            data,
            size_compressed,
            &data_uncompressed,
            NULL))
        {
            assert(0);
        }
        assert(data_uncompressed != NULL);
        free(data);
        data = data_uncompressed;
    }

    return data;
}

static VirtualFile *npa_read_file(void *_context)
{
    NpaUnpackContext *unpack_context = (NpaUnpackContext*)_context;
    VirtualFile *file = virtual_file_create();
    assert(file != NULL);

    size_t file_name_length;
    char *file_name = npa_read_file_name(unpack_context, &file_name_length);
    char *file_name_utf8;
    convert_encoding(
        file_name, file_name_length,
        &file_name_utf8, NULL,
        "cp932", "utf-8");
    assert(file_name_utf8 != NULL);
    virtual_file_set_name(file, file_name_utf8);

    NpaFileType file_type = (NpaFileType)io_read_u8(unpack_context->arc_io);
    io_skip(unpack_context->arc_io, 4);
    uint32_t offset = io_read_u32_le(unpack_context->arc_io);
    uint32_t size_compressed = io_read_u32_le(unpack_context->arc_io);
    uint32_t size_original = io_read_u32_le(unpack_context->arc_io);
    offset += unpack_context->table_offset + unpack_context->header->table_size;

    if (file_type == NPA_FILE_TYPE_DIRECTORY)
    {
        log_info("NPA: Empty directory - ignoring.");
        virtual_file_destroy(file);
        return NULL;
    }
    else if (file_type != NPA_FILE_TYPE_FILE)
    {
        virtual_file_destroy(file);
        log_error("NPA: Unknown file type: %d", file_type);
        return NULL;
    }

    size_t old_pos = io_tell(unpack_context->arc_io);
    io_seek(unpack_context->arc_io, offset);
    char *file_data = npa_read_file_data(
        size_compressed,
        size_original,
        file_name,
        file_name_length,
        unpack_context);
    io_write_string(file->io, file_data, size_original);
    free(file_data);
    free(file_name);
    io_seek(unpack_context->arc_io, old_pos);

    return file;
}

static bool npa_unpack(
    Archive *archive,
    IO *arc_io,
    OutputFiles *output_files)
{
    NpaArchiveContext *archive_context = (NpaArchiveContext*)archive->data;
    assert(archive_context != NULL);
    if (!npa_check_magic(arc_io))
    {
        log_error("NPA: Not a NPA archive");
        return false;
    }

    if (archive_context->filter == NULL)
    {
        log_error("NPA: No plugin selected");
        return false;
    }

    NpaHeader *header = npa_read_header(arc_io);
    NpaUnpackContext unpack_context;
    unpack_context.arc_io = arc_io;
    unpack_context.header = header;
    unpack_context.archive_context = archive_context;
    unpack_context.table_offset = io_tell(arc_io);
    size_t file_pos;
    for (file_pos = 0; file_pos < header->total_count; file_pos ++)
    {
        unpack_context.file_pos = file_pos;
        output_files_save(output_files, &npa_read_file, &unpack_context);
    }
    free(header);
    return true;
}

Archive *npa_archive_create()
{
    Archive *archive = archive_create();
    archive->add_cli_help = &npa_add_cli_help;
    archive->parse_cli_options = &npa_parse_cli_options;
    archive->unpack = &npa_unpack;
    archive->cleanup = &npa_cleanup;
    return archive;
}
