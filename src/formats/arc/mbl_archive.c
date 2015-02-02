// MBL archive
//
// Company:   Ivory
// Engine:    MarbleEngine
// Extension: .mbl
//
// Known games:
// - Wanko to Kurasou

#include <stdlib.h>
#include <string.h>
#include "assert_ex.h"
#include "formats/arc/mbl_archive.h"
#include "formats/gfx/prs_converter.h"
#include "logger.h"
#include "string_ex.h"

typedef struct
{
    Converter *prs_converter;
    IO *arc_io;
    size_t name_length;
} UnpackContext;

static int mbl_check_version(
    IO *arc_io,
    size_t initial_position,
    uint32_t file_count,
    uint32_t name_length)
{
    io_seek(arc_io, initial_position + file_count * (name_length + 8));
    io_skip(arc_io, -8);
    uint32_t last_file_offset = io_read_u32_le(arc_io);
    uint32_t last_file_size = io_read_u32_le(arc_io);
    return last_file_offset + last_file_size == io_size(arc_io);
}

static int mbl_get_version(IO *arc_io)
{
    uint32_t file_count = io_read_u32_le(arc_io);
    if (mbl_check_version(arc_io, 4, file_count, 16))
    {
        io_seek(arc_io, 0);
        return 1;
    }

    io_seek(arc_io, 4);
    uint32_t name_length = io_read_u32_le(arc_io);
    if (mbl_check_version(arc_io, 8, file_count, name_length))
    {
        io_seek(arc_io, 0);
        return 2;
    }

    return -1;
}

static VirtualFile *mbl_read_file(void *_context)
{
    UnpackContext *context = (UnpackContext*)_context;
    assert_not_null(context);
    VirtualFile *file = virtual_file_create();

    size_t old_pos = io_tell(context->arc_io);
    char *tmp_name = NULL;
    io_read_until_zero(context->arc_io, &tmp_name, NULL);
    assert_not_null(tmp_name);

    char *decoded_name;
    assert_that(convert_encoding(
        tmp_name, strlen(tmp_name),
        &decoded_name, NULL,
        "sjis", "utf-8"));
    virtual_file_set_name(file, decoded_name);
    free(decoded_name);

    free(tmp_name);
    io_seek(context->arc_io, old_pos + context->name_length);

    size_t offset = io_read_u32_le(context->arc_io);
    size_t size = io_read_u32_le(context->arc_io);
    if (offset + size > io_size(context->arc_io))
    {
        log_error("Bad offset to file");
        return false;
    }

    old_pos = io_tell(context->arc_io);
    io_seek(context->arc_io, offset);
    io_write_string_from_io(file->io, context->arc_io, size);
    io_seek(context->arc_io, old_pos);

    converter_try_decode(context->prs_converter, file);

    return file;
}

static bool mbl_unpack(
    Archive *archive,
    IO *arc_io,
    OutputFiles *output_files)
{
    size_t i;
    int version = mbl_get_version(arc_io);
    if (version == -1)
    {
        log_error("Not a MBL archive");
        return false;
    }
    log_info("Version: %d", version);

    uint32_t file_count = io_read_u32_le(arc_io);
    uint32_t name_length = version == 2 ? io_read_u32_le(arc_io) : 16;
    UnpackContext unpack_context;
    unpack_context.prs_converter = (Converter*)archive->data;
    unpack_context.arc_io = arc_io;
    unpack_context.name_length = name_length;
    for (i = 0; i < file_count; i ++)
    {
        output_files_save(output_files, &mbl_read_file, &unpack_context);
    }

    return true;
}

void mbl_cleanup(Archive *archive)
{
    assert_not_null(archive);
    converter_destroy((Converter*)archive->data);
}

Archive *mbl_archive_create()
{
    Archive *archive = archive_create();
    archive->unpack = &mbl_unpack;
    archive->cleanup = &mbl_cleanup;
    archive->data = (void*)prs_converter_create();
    assert_not_null(archive->data);
    return archive;
}
