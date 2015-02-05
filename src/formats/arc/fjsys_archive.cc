// FJSYS archive
//
// Engine:    -
// Company:   various
// Extension: none
//
// Known games:
// - Sono Hanabira ni Kuchizuke o 1
// - Sono Hanabira ni Kuchizuke o 2
// - Sono Hanabira ni Kuchizuke o 3
// - Sono Hanabira ni Kuchizuke o 4
// - Sono Hanabira ni Kuchizuke o 5
// - Sono Hanabira ni Kuchizuke o 6
// - Sono Hanabira ni Kuchizuke o 7
// - Sono Hanabira ni Kuchizuke o 8
// - Sono Hanabira ni Kuchizuke o 9
// - Sono Hanabira ni Kuchizuke o 10
// - Sono Hanabira ni Kuchizuke o 11

#include <cassert>
#include <cstring>
#include "formats/arc/fjsys_archive.h"
#include "formats/gfx/mgd_converter.h"
#include "logger.h"

static const char *fjsys_magic = "FJSYS\x00\x00\x00";
static const size_t fjsys_magic_length = 8;

typedef struct
{
    size_t header_size;
    size_t file_names_size;
    size_t file_count;
} FjsysHeader;

typedef struct
{
    Converter *mgd_converter;
} FjsysArchiveContext;

typedef struct
{
    FjsysHeader *header;
    FjsysArchiveContext *archive_context;
    IO *arc_io;
} FjsysUnpackContext;

static void fjsys_add_cli_help(
    Archive *archive,
    ArgParser &arg_parser)
{
    FjsysArchiveContext *archive_context = (FjsysArchiveContext*)archive->data;
    converter_add_cli_help(archive_context->mgd_converter, arg_parser);
}

static void fjsys_parse_cli_options(
    Archive *archive,
    ArgParser &arg_parser)
{
    FjsysArchiveContext *archive_context = (FjsysArchiveContext*)archive->data;
    converter_parse_cli_options(archive_context->mgd_converter, arg_parser);
}

static bool fjsys_check_magic(IO *arc_io)
{
    char magic[fjsys_magic_length];
    io_read_string(arc_io, magic, fjsys_magic_length);
    return memcmp(magic, fjsys_magic, fjsys_magic_length) == 0;
}

static FjsysHeader *fjsys_read_header(IO *arc_io)
{
    FjsysHeader *header = new FjsysHeader;
    assert(header != nullptr);
    header->header_size = io_read_u32_le(arc_io);
    header->file_names_size = io_read_u32_le(arc_io);
    header->file_count = io_read_u32_le(arc_io);
    io_skip(arc_io, 64);
    return header;
}

static VirtualFile *fjsys_read_file(void *context)
{
    VirtualFile *file = virtual_file_create();
    FjsysUnpackContext *unpack_context = (FjsysUnpackContext*)context;
    size_t file_name_offset = io_read_u32_le(unpack_context->arc_io);
    size_t data_size = io_read_u32_le(unpack_context->arc_io);
    size_t data_offset = io_read_u64_le(unpack_context->arc_io);
    size_t old_pos = io_tell(unpack_context->arc_io);
    size_t file_names_start = unpack_context->header->header_size
        - unpack_context->header->file_names_size;

    io_seek(
        unpack_context->arc_io,
        file_name_offset + file_names_start);
    char *file_name = nullptr;
    io_read_until_zero(unpack_context->arc_io, &file_name, nullptr);
    assert(file_name != nullptr);
    virtual_file_set_name(file, file_name);
    delete []file_name;

    io_seek(unpack_context->arc_io, data_offset);
    io_write_string_from_io(file->io, unpack_context->arc_io, data_size);
    converter_try_decode(unpack_context->archive_context->mgd_converter, file);

    io_seek(unpack_context->arc_io, old_pos);
    return file;
}

static bool fjsys_unpack(
    Archive *archive,
    IO *arc_io,
    OutputFiles *output_files)
{
    FjsysArchiveContext *archive_context = (FjsysArchiveContext*)archive->data;
    assert(archive_context != nullptr);
    if (!fjsys_check_magic(arc_io))
    {
        log_error("FJSYS: Not a FJSYS archive");
        return false;
    }

    FjsysHeader *header = fjsys_read_header(arc_io);
    assert(header != nullptr);

    FjsysUnpackContext unpack_context;
    unpack_context.header = header;
    unpack_context.archive_context = archive_context;
    unpack_context.arc_io = arc_io;

    size_t i;
    for (i = 0; i < header->file_count; i ++)
        output_files_save(output_files, &fjsys_read_file, &unpack_context);

    delete header;
    return true;
}

static void fjsys_cleanup(Archive *archive)
{
    FjsysArchiveContext *archive_context = (FjsysArchiveContext*)archive->data;
    converter_destroy(archive_context->mgd_converter);
}

Archive *fjsys_archive_create()
{
    Archive *archive = archive_create();
    archive->add_cli_help = &fjsys_add_cli_help;
    archive->parse_cli_options = &fjsys_parse_cli_options;
    archive->unpack = &fjsys_unpack;
    archive->cleanup = &fjsys_cleanup;

    FjsysArchiveContext *context = new FjsysArchiveContext;
    assert(context != nullptr);
    context->mgd_converter = mgd_converter_create();
    archive->data = context;

    return archive;
}
