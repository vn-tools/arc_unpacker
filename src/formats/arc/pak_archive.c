// PAK2 archive
//
// Engine:    -
// Company:   Nitroplus
// Extension: .pak
//
// Known games:
// - Saya no Uta

#include <stdlib.h>
#include <string.h>
#include "assert_ex.h"
#include "formats/arc/pak_archive.h"
#include "logger.h"
#include "string_ex.h"

static const char *pak_magic = "\x02\x00\x00\x00";
static const size_t pak_magic_length = 4;

typedef struct
{
    IO *arc_io;
    IO *table_io;
    size_t offset_to_files;
} PakUnpackContext;

static bool pak_check_magic(IO *arc_io)
{
    char magic[pak_magic_length];
    io_read_string(arc_io, magic, pak_magic_length);
    return memcmp(magic, pak_magic, pak_magic_length) == 0;
}

static char *pak_read_zlib(
    IO *arc_io,
    size_t size_compressed,
    size_t *size_uncompressed)
{
    char *data_compressed = (char*)malloc(size_compressed);
    assert_not_null(data_compressed);

    assert_that(io_read_string(arc_io, data_compressed, size_compressed));
    char *data_uncompressed;
    assert_that(zlib_inflate(
        data_compressed,
        size_compressed,
        &data_uncompressed,
        size_uncompressed));
    assert_not_null(data_uncompressed);
    free(data_compressed);

    return data_uncompressed;
}

static VirtualFile *pak_read_file(void *context)
{
    PakUnpackContext *unpack_context = (PakUnpackContext*)context;
    VirtualFile *file = virtual_file_create();
    assert_not_null(file);

    size_t file_name_length = io_read_u32_le(unpack_context->table_io);
    char *file_name = (char*)malloc(file_name_length);
    assert_not_null(file_name);
    io_read_string(unpack_context->table_io, file_name, file_name_length);

    char *file_name_utf8 = NULL;
    assert_that(convert_encoding(
        file_name, file_name_length,
        &file_name_utf8, NULL,
        "cp932", "utf-8"));
    assert_not_null(file_name_utf8);
    virtual_file_set_name(file, file_name_utf8);
    free(file_name_utf8);

    size_t offset = io_read_u32_le(unpack_context->table_io);
    size_t size_original = io_read_u32_le(unpack_context->table_io);
    io_skip(unpack_context->table_io, 4);
    size_t flags = io_read_u32_le(unpack_context->table_io);
    size_t size_compressed = io_read_u32_le(unpack_context->table_io);
    offset += unpack_context->offset_to_files;

    io_seek(unpack_context->arc_io, offset);
    if (flags > 0)
    {
        size_t size_uncompressed = 0;
        char *data_uncompressed = pak_read_zlib(
            unpack_context->arc_io, size_compressed, &size_uncompressed);
        assert_not_null(data_uncompressed);
        io_write_string(file->io, data_uncompressed, size_original);
        free(data_uncompressed);
        if (size_uncompressed != size_original)
        {
            virtual_file_destroy(file);
            log_error("Bad file size");
            return NULL;
        }
    }
    else
    {
        io_write_string_from_io(
            file->io, unpack_context->arc_io, size_original);
    }

    free(file_name);
    return file;
}

static bool pak_unpack(
    __attribute__((unused)) Archive *archive,
    IO *arc_io,
    OutputFiles *output_files)
{
    if (!pak_check_magic(arc_io))
    {
        log_error("Not a PAK archive");
        return false;
    }

    uint32_t file_count = io_read_u32_le(arc_io);
    uint32_t table_size_original = io_read_u32_le(arc_io);
    uint32_t table_size_compressed = io_read_u32_le(arc_io);
    io_skip(arc_io, 0x104);

    char *table = pak_read_zlib(arc_io, table_size_compressed, NULL);
    assert_not_null(table);

    IO *table_io = io_create_from_buffer(table, table_size_original);
    assert_not_null(table_io);
    free(table);

    size_t i;
    PakUnpackContext unpack_context;
    unpack_context.arc_io = arc_io;
    unpack_context.table_io = table_io;
    unpack_context.offset_to_files = io_tell(arc_io);
    for (i = 0; i < file_count; i ++)
    {
        output_files_save(output_files, &pak_read_file, &unpack_context);
    }

    io_destroy(table_io);
    return true;
}

Archive *pak_archive_create()
{
    Archive *archive = archive_create();
    archive->unpack = &pak_unpack;
    return archive;
}
