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
    IO *arc_io;
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

static int xp3_detect_version(IO *io)
{
    int version = 1;
    size_t old_pos = io_tell(io);
    assert_not_null(io);
    io_seek(io, 19);
    if (io_read_u32_le(io) == 1)
        version = 2;
    io_seek(io, old_pos);
    return version;
}

static bool xp3_check_magic(IO *io, const char *expected_magic, size_t length)
{
    char *magic = io_read_string(io, length);
    bool ok = memcmp(magic, expected_magic, length) == 0;
    free(magic);
    return ok;
}

static uint64_t xp3_get_table_offset(IO *io, int version)
{
    if (version == 1)
        return io_read_u64_le(io);
    uint64_t additional_header_offset = io_read_u64_le(io);
    uint32_t minor_version = io_read_u32_le(io);
    if (minor_version != 1)
    {
        log_error("Unexpected XP3 version: %d", minor_version);
        return false;
    }

    io_seek(io, additional_header_offset & 0xfffffffff);
    io_skip(io, 1); // flags?
    io_skip(io, 8); // table size
    return io_read_u64_le(io);
}

static IO *xp3_read_raw_table(IO *io)
{
    assert_not_null(io);
    bool use_zlib = io_read_u8(io);
    const uint64_t table_size_compressed = io_read_u64_le(io);
    const uint64_t table_size_original = use_zlib
        ? io_read_u64_le(io)
        : table_size_compressed;

    char *table_data = io_read_string(io, table_size_compressed);
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
    __attribute__((unused)) uint64_t file_size_original = io_read_u64_le(table_io);
    __attribute__((unused)) uint64_t file_size_compressed = io_read_u64_le(table_io);
    size_t name_length = io_read_u16_le(table_io);

    char *name_utf16 = io_read_string(table_io, name_length * 2);
    char *name_utf8;
    assert_that(convert_encoding(
        name_utf16, name_length * 2,
        &name_utf8, NULL,
        "UTF-16LE", "UTF-8"));
    assert_not_null(name_utf8);
    vf_set_name(target_file, name_utf8);
    free(name_utf16);
    free(name_utf8);
    assert_equali(name_length * 2 + 22, info_chunk_size);
    return true;
}

static bool xp3_read_segm_chunk(
    IO *table_io,
    IO *arc_io,
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
    io_seek(arc_io, data_offset);
    char *data = io_read_string(arc_io, data_size_compressed);
    assert_not_null(data);
    bool use_zlib = segm_flags & 7;
    if (use_zlib)
    {
        char *data_uncompressed;
        size_t data_size_uncompressed;
        assert_that(zlib_inflate(
            data,
            data_size_compressed,
            &data_uncompressed,
            &data_size_uncompressed));
        assert_equali(data_size_original, data_size_uncompressed);
        free(data);
        data = data_uncompressed;
    }

    size_t full_data_size = vf_get_size(target_file) + data_size_original;
    char *full_data = (char*)malloc(full_data_size);
    assert_not_null(full_data);
    memcpy(full_data, vf_get_data(target_file), vf_get_size(target_file));
    memcpy(full_data + vf_get_size(target_file), data, data_size_original);
    assert_that(vf_set_data(target_file, full_data, full_data_size));
    free(full_data);

    free(data);
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
    IO *arc_io = ((Xp3Context*)context)->arc_io;
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
        if (!xp3_read_segm_chunk(table_io, arc_io, target_file))
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

static bool xp3_unpack(Archive *archive, IO *io, OutputFiles *output_files)
{
    assert_not_null(archive);
    assert_not_null(io);
    assert_not_null(output_files);

    if (!xp3_check_magic(io, xp3_magic, xp3_magic_length))
    {
        log_error("Not an XP3 archive");
        return false;
    }

    int version = xp3_detect_version(io);
    log_info("Version: %d", version);

    uint64_t table_offset = xp3_get_table_offset(io, version);
    io_seek(io, table_offset & 0xffffffff);
    IO *table_io = xp3_read_raw_table(io);
    assert_not_null(table_io);

    Xp3Context context;
    context.arc_io = io;
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
