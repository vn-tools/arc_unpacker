#include <stdlib.h>
#include <string.h>
#include "assert_ex.h"
#include "formats/arc/xp3_archive.h"
#include "io.h"
#include "logger.h"
#include "string_ex.h"

typedef struct
{
    IO *table_io;
    IO *arc_file;
} Xp3Context;

static const char *xp3_magic = "XP3\r\n\x20\x0a\x1a\x8b\x67\x01";
static const size_t xp3_magic_length = 11;

static const char *file_magic = "File";
static const size_t file_magic_length = 4;

static const char *adlr_magic = "adlr";
static const size_t adlr_magic_length = 4;

static const char *info_magic = "info";
static const size_t info_magic_length = 4;

static const char *segm_magic = "segm";
static const size_t segm_magic_length = 4;

static int xp3_detect_version(IO *arc_file)
{
    int version = 1;
    size_t old_pos = io_tell(arc_file);
    assert_not_null(arc_file);
    io_seek(arc_file, 19);
    if (io_read_u32_le(arc_file) == 1)
        version = 2;
    io_seek(arc_file, old_pos);
    return version;
}

static bool xp3_check_magic(
    IO *arc_file,
    const char *expected_magic,
    size_t length)
{
    char *magic = (char*)malloc(length);
    assert_not_null(magic);
    io_read_string(arc_file, magic, length);
    bool ok = memcmp(magic, expected_magic, length) == 0;
    free(magic);
    return ok;
}

static uint64_t xp3_get_table_offset(IO *arc_file, int version)
{
    if (version == 1)
        return io_read_u64_le(arc_file);
    uint64_t additional_header_offset = io_read_u64_le(arc_file);
    uint32_t minor_version = io_read_u32_le(arc_file);
    if (minor_version != 1)
    {
        log_error("Unexpected XP3 version: %d", minor_version);
        return false;
    }

    io_seek(arc_file, additional_header_offset & 0xfffffffff);
    io_skip(arc_file, 1); // flags?
    io_skip(arc_file, 8); // table size
    return io_read_u64_le(arc_file);
}

static IO *xp3_read_raw_table(IO *arc_file)
{
    assert_not_null(arc_file);
    bool use_zlib = io_read_u8(arc_file);
    const uint64_t table_size_compressed = io_read_u64_le(arc_file);
    const uint64_t table_size_original = use_zlib
        ? io_read_u64_le(arc_file)
        : table_size_compressed;

    char *table_data = (char*)malloc(table_size_compressed);
    io_read_string(arc_file, table_data, table_size_compressed);
    if (use_zlib)
    {
        char *table_data_uncompressed;
        size_t table_size_uncompressed;
        assert_that(zlib_inflate(
            table_data,
            table_size_compressed,
            &table_data_uncompressed,
            &table_size_uncompressed));
        assert_not_null(table_data_uncompressed);
        assert_equali(table_size_original, table_size_uncompressed);
        free(table_data);
        table_data = table_data_uncompressed;
    }

    IO *table_io = io_create_from_buffer(
        table_data,
        table_size_original);
    free(table_data);
    return table_io;
}

static bool xp3_read_info_chunk(IO *table_io, VirtualFile *target_file)
{
    if (!xp3_check_magic(table_io, info_magic, info_magic_length))
    {
        log_error("Expected INFO chunk");
        return false;
    }
    uint64_t info_chunk_size = io_read_u64_le(table_io);

    __attribute__((unused)) uint32_t info_flags = io_read_u32_le(table_io);
    __attribute__((unused)) uint64_t file_size_original
        = io_read_u64_le(table_io);
    __attribute__((unused)) uint64_t file_size_compressed
        = io_read_u64_le(table_io);

    size_t name_length = io_read_u16_le(table_io);

    char *name_utf16 = (char*)malloc(name_length * 2);
    assert_not_null(name_utf16);
    io_read_string(table_io, name_utf16, name_length * 2);

    char *name_utf8;
    assert_that(convert_encoding(
        name_utf16, name_length * 2,
        &name_utf8, NULL,
        "UTF-16LE", "UTF-8"));
    assert_not_null(name_utf8);
    vf_set_name(target_file, name_utf8);
    free(name_utf8);

    free(name_utf16);
    assert_equali(name_length * 2 + 22, info_chunk_size);
    return true;
}

static bool xp3_read_segm_chunk(
    IO *table_io,
    IO *arc_file,
    VirtualFile *target_file)
{
    if (!xp3_check_magic(table_io, segm_magic, segm_magic_length))
    {
        io_skip(table_io, -segm_magic_length);
        return false;
    }
    uint64_t segm_chunk_size = io_read_u64_le(table_io);
    assert_equali(28, segm_chunk_size);

    uint32_t segm_flags = io_read_u32_le(table_io);
    uint64_t data_offset = io_read_u64_le(table_io);
    uint64_t data_size_original = io_read_u64_le(table_io);
    uint64_t data_size_compressed = io_read_u64_le(table_io);
    io_seek(arc_file, data_offset);

    bool use_zlib = segm_flags & 7;
    if (use_zlib)
    {
        char *data = (char*)malloc(data_size_compressed);
        assert_not_null(data);
        io_read_string(arc_file, data, data_size_compressed);

        char *data_uncompressed;
        size_t data_size_uncompressed;
        assert_that(zlib_inflate(
            data,
            data_size_compressed,
            &data_uncompressed,
            &data_size_uncompressed));
        assert_equali(data_size_original, data_size_uncompressed);
        free(data);

        io_write_string(target_file->io, data_uncompressed, data_size_original);
        free(data_uncompressed);
    }
    else
    {
        io_write_string_from_io(target_file->io, arc_file, data_size_original);
    }

    return true;
}

static uint32_t xp3_read_adlr_chunk(IO *table_io, uint32_t *encryption_key)
{
    if (!xp3_check_magic(table_io, adlr_magic, adlr_magic_length))
    {
        log_error("Expected ADLR chunk");
        return false;
    }
    uint64_t adlr_chunk_size = io_read_u64_le(table_io);
    assert_equali(4, adlr_chunk_size);

    *encryption_key = io_read_u32_le(table_io);
    return true;
}

static VirtualFile *xp3_read_file(void *context)
{
    IO *arc_file = ((Xp3Context*)context)->arc_file;
    IO *table_io = ((Xp3Context*)context)->table_io;
    VirtualFile *target_file = vf_create();

    if (!xp3_check_magic(table_io, file_magic, file_magic_length))
    {
        log_error("Expected FILE chunk");
        io_destroy(table_io);
        return NULL;
    }
    uint64_t file_chunk_size = io_read_u64_le(table_io);
    size_t file_chunk_start_offset = io_tell(table_io);

    if (!xp3_read_info_chunk(table_io, target_file))
    {
        io_destroy(table_io);
        return false;
    }

    while (true)
    {
        if (!xp3_read_segm_chunk(table_io, arc_file, target_file))
            break;
    }

    uint32_t encryption_key;
    if (!xp3_read_adlr_chunk(table_io, &encryption_key))
    {
        io_destroy(table_io);
        return false;
    }

    assert_equali(io_tell(table_io) - file_chunk_start_offset, file_chunk_size);

    return target_file;
}

static bool xp3_unpack(
    Archive *archive,
    IO *arc_file,
    OutputFiles *output_files)
{
    assert_not_null(archive);
    assert_not_null(arc_file);
    assert_not_null(output_files);

    if (!xp3_check_magic(arc_file, xp3_magic, xp3_magic_length))
    {
        log_error("Not an XP3 archive");
        return false;
    }

    int version = xp3_detect_version(arc_file);
    log_info("Version: %d", version);

    uint64_t table_offset = xp3_get_table_offset(arc_file, version);
    io_seek(arc_file, table_offset & 0xffffffff);
    IO *table_io = xp3_read_raw_table(arc_file);
    assert_not_null(table_io);

    Xp3Context context;
    context.arc_file = arc_file;
    context.table_io = table_io;
    while (io_tell(table_io) < io_size(table_io))
        output_files_save(output_files, &xp3_read_file, &context);

    io_destroy(table_io);
    return true;
}

Archive *xp3_archive_create()
{
    Archive *archive = archive_create();
    archive->unpack = &xp3_unpack;
    return archive;
}
