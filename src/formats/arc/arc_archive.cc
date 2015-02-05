// ARC archive
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: .arc
//
// Known games:
// - Higurashi No Naku Koro Ni

#include <cassert>
#include <cstring>
#include "formats/arc/arc_archive.h"
#include "formats/gfx/cbg_converter.h"
#include "logger.h"

static const char *arc_magic = "PackFile    ";
static const size_t arc_magic_length = 12;

typedef struct
{
    IO *arc_io;
    Converter *cbg_converter;
    size_t file_count;
} ArcUnpackContext;

static bool arc_check_magic(IO *arc_io)
{
    char magic[arc_magic_length];
    io_read_string(arc_io, magic, arc_magic_length);
    return memcmp(magic, arc_magic, arc_magic_length) == 0;
}

static VirtualFile *arc_read_file(void *_context)
{
    ArcUnpackContext *context = (ArcUnpackContext*)_context;
    assert(context != NULL);

    VirtualFile *file = virtual_file_create();

    size_t old_pos = io_tell(context->arc_io);
    char *tmp_name;
    io_read_until_zero(context->arc_io, &tmp_name, NULL);
    assert(tmp_name != NULL);
    virtual_file_set_name(file, tmp_name);
    delete []tmp_name;
    io_seek(context->arc_io, old_pos + 16);

    size_t offset = io_read_u32_le(context->arc_io);
    size_t size = io_read_u32_le(context->arc_io);
    offset += arc_magic_length + 4 + context->file_count * 32;
    io_skip(context->arc_io, 8);
    if (offset + size > io_size(context->arc_io))
    {
        log_error("ARC: Bad offset to file");
        virtual_file_destroy(file);
        return NULL;
    }

    old_pos = io_tell(context->arc_io);
    if (!io_seek(context->arc_io, offset))
    {
        log_error("ARC: Failed to seek to file");
        virtual_file_destroy(file);
        return NULL;
    }
    io_write_string_from_io(file->io, context->arc_io, size);
    io_seek(context->arc_io, old_pos);

    converter_try_decode(context->cbg_converter, file);

    return file;
}

static bool arc_unpack(
    Archive *archive,
    IO *arc_io,
    OutputFiles *output_files)
{
    if (!arc_check_magic(arc_io))
    {
        log_error("ARC: Not an ARC archive");
        return false;
    }

    size_t file_count = io_read_u32_le(arc_io);
    if (file_count * 32 > io_size(arc_io))
    {
        log_error("ARC: Bad file count");
        return false;
    }

    ArcUnpackContext unpack_context;
    unpack_context.cbg_converter = (Converter*)archive->data;
    unpack_context.arc_io = arc_io;
    unpack_context.file_count = file_count;
    size_t i;
    for (i = 0; i < file_count; i ++)
    {
        output_files_save(output_files, &arc_read_file, &unpack_context);
    }
    return true;
}

void arc_cleanup(Archive *archive)
{
    assert(archive != NULL);
    converter_destroy((Converter*)archive->data);
}

Archive *arc_archive_create()
{
    Archive *archive = archive_create();
    archive->unpack = &arc_unpack;
    archive->cleanup = &arc_cleanup;
    archive->data = (void*)cbg_converter_create();
    return archive;
}
