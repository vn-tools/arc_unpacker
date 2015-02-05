// XP3 archive
//
// Company:   -
// Engine:    Kirikiri
// Extension: .xp3
//
// Known games:
// - Fate/Stay Night
// - Fate/Hollow Ataraxia
// - Sono Hanabira ni Kuchizuke o 12
// - Sharin no Kuni, Himawari no Shoujo
// - Comyu Kuroi Ryuu to Yasashii Oukoku

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "formats/arc/xp3_archive.h"
#include "io.h"
#include "logger.h"
#include "string_ex.h"

typedef struct
{
    IO *table_io;
    IO *arc_io;
} Xp3UnpackContext;

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

static int xp3_detect_version(IO *arc_io)
{
    assert(arc_io != NULL);
    int version = 1;
    size_t old_pos = io_tell(arc_io);
    io_seek(arc_io, 19);
    if (io_read_u32_le(arc_io) == 1)
        version = 2;
    io_seek(arc_io, old_pos);
    return version;
}

static bool xp3_check_magic(
    IO *arc_io,
    const char *expected_magic,
    size_t length)
{
    char *magic = (char*)malloc(length);
    assert(magic != NULL);
    io_read_string(arc_io, magic, length);
    bool ok = memcmp(magic, expected_magic, length) == 0;
    free(magic);
    return ok;
}

static uint64_t xp3_get_table_offset(IO *arc_io, int version)
{
    assert(arc_io != NULL);
    if (version == 1)
        return io_read_u64_le(arc_io);
    uint64_t additional_header_offset = io_read_u64_le(arc_io);
    uint32_t minor_version = io_read_u32_le(arc_io);
    if (minor_version != 1)
    {
        log_error("XP3: Unexpected XP3 version: %d", minor_version);
        return false;
    }

    io_seek(arc_io, additional_header_offset & 0xfffffffff);
    io_skip(arc_io, 1); // flags?
    io_skip(arc_io, 8); // table size
    return io_read_u64_le(arc_io);
}

static IO *xp3_read_raw_table(IO *arc_io)
{
    assert(arc_io != NULL);

    IO *table_io = NULL;
    bool use_zlib = io_read_u8(arc_io);
    const uint64_t table_size_compressed = io_read_u64_le(arc_io);
    const uint64_t table_size_original = use_zlib
        ? io_read_u64_le(arc_io)
        : table_size_compressed;

    char *table_data = (char*)malloc(table_size_compressed);
    if (!table_data)
    {
        log_error("XP3: Failed to allocate memory for table");
    }
    else
    {
        if (!io_read_string(arc_io, table_data, table_size_compressed))
        {
            log_error("XP3: Failed to read raw table data");
        }
        else
        {
            if (use_zlib)
            {
                char *table_data_uncompressed;
                size_t table_size_uncompressed;
                if (!zlib_inflate(
                    table_data,
                    table_size_compressed,
                    &table_data_uncompressed,
                    &table_size_uncompressed))
                {
                    log_error("XP3: Failed to decompress zlib stream");
                    return NULL;
                }
                assert(table_data_uncompressed != NULL);
                assert(table_size_original == table_size_uncompressed);
                free(table_data);
                table_data = table_data_uncompressed;
            }
            table_io = io_create_from_buffer(
                table_data,
                table_size_original);
        }
        free(table_data);
    }

    return table_io;
}

static bool xp3_read_info_chunk(IO *table_io, VirtualFile *target_file)
{
    assert(table_io != NULL);
    assert(target_file != NULL);

    if (!xp3_check_magic(table_io, info_magic, info_magic_length))
    {
        log_error("XP3: Expected INFO chunk");
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
    assert(name_utf16 != NULL);
    io_read_string(table_io, name_utf16, name_length * 2);

    char *name_utf8;
    if (!convert_encoding(
        name_utf16, name_length * 2,
        &name_utf8, NULL,
        "UTF-16LE", "UTF-8"))
    {
        assert(0);
    }
    assert(name_utf8 != NULL);
    virtual_file_set_name(target_file, name_utf8);
    free(name_utf8);

    free(name_utf16);
    assert(name_length * 2 + 22 == info_chunk_size);
    return true;
}

static bool xp3_read_segm_chunk(
    IO *table_io,
    IO *arc_io,
    VirtualFile *target_file)
{
    assert(table_io != NULL);
    assert(arc_io != NULL);
    assert(target_file != NULL);

    if (!xp3_check_magic(table_io, segm_magic, segm_magic_length))
    {
        io_skip(table_io, -segm_magic_length);
        return false;
    }
    uint64_t segm_chunk_size = io_read_u64_le(table_io);
    assert(28 == segm_chunk_size);

    uint32_t segm_flags = io_read_u32_le(table_io);
    uint64_t data_offset = io_read_u64_le(table_io);
    uint64_t data_size_original = io_read_u64_le(table_io);
    uint64_t data_size_compressed = io_read_u64_le(table_io);
    io_seek(arc_io, data_offset);

    bool use_zlib = segm_flags & 7;
    if (use_zlib)
    {
        char *data = (char*)malloc(data_size_compressed);
        assert(data != NULL);
        io_read_string(arc_io, data, data_size_compressed);

        char *data_uncompressed;
        size_t data_size_uncompressed;
        if (!zlib_inflate(
            data,
            data_size_compressed,
            &data_uncompressed,
            &data_size_uncompressed))
        {
            assert(0);
        }
        assert(data_size_original == data_size_uncompressed);
        free(data);

        io_write_string(target_file->io, data_uncompressed, data_size_original);
        free(data_uncompressed);
    }
    else
    {
        io_write_string_from_io(target_file->io, arc_io, data_size_original);
    }

    return true;
}

static uint32_t xp3_read_adlr_chunk(IO *table_io, uint32_t *encryption_key)
{
    assert(table_io != NULL);
    assert(encryption_key != NULL);

    if (!xp3_check_magic(table_io, adlr_magic, adlr_magic_length))
    {
        log_error("XP3: Expected ADLR chunk");
        return false;
    }
    uint64_t adlr_chunk_size = io_read_u64_le(table_io);
    assert(4 == adlr_chunk_size);

    *encryption_key = io_read_u32_le(table_io);
    return true;
}

static VirtualFile *xp3_read_file(void *context)
{
    assert(context != NULL);

    IO *arc_io = ((Xp3UnpackContext*)context)->arc_io;
    IO *table_io = ((Xp3UnpackContext*)context)->table_io;
    VirtualFile *target_file = virtual_file_create();

    if (!xp3_check_magic(table_io, file_magic, file_magic_length))
    {
        log_error("XP3: Expected FILE chunk");
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

    assert(io_tell(table_io) - file_chunk_start_offset == file_chunk_size);
    return target_file;
}

static bool xp3_unpack(
    Archive *archive,
    IO *arc_io,
    OutputFiles *output_files)
{
    assert(archive != NULL);
    assert(arc_io != NULL);
    assert(output_files != NULL);

    if (!xp3_check_magic(arc_io, xp3_magic, xp3_magic_length))
    {
        log_error("XP3: Not an XP3 archive");
        return false;
    }

    int version = xp3_detect_version(arc_io);
    log_info("XP3: Version: %d", version);

    uint64_t table_offset = xp3_get_table_offset(arc_io, version);
    io_seek(arc_io, (uint32_t)table_offset);
    IO *table_io = xp3_read_raw_table(arc_io);

    bool result;
    if (table_io == NULL)
    {
        log_error("XP3: Failed to read file table");
        result = false;
    }
    else
    {
        Xp3UnpackContext unpack_context;
        unpack_context.arc_io = arc_io;
        unpack_context.table_io = table_io;
        while (io_tell(table_io) < io_size(table_io))
            output_files_save(output_files, &xp3_read_file, &unpack_context);
        io_destroy(table_io);
        result = true;
    }

    return result;
}

Archive *xp3_archive_create()
{
    Archive *archive = archive_create();
    archive->unpack = &xp3_unpack;
    return archive;
}
