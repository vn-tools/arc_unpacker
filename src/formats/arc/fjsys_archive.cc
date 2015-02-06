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

namespace
{
    const char *fjsys_magic = "FJSYS\x00\x00\x00";
    const size_t fjsys_magic_length = 8;

    typedef struct
    {
        size_t header_size;
        size_t file_names_size;
        size_t file_count;
    } FjsysHeader;

    typedef struct
    {
        FjsysHeader *header;
        MgdConverter *mgd_converter;
        IO *arc_io;
    } FjsysUnpackContext;

    bool fjsys_check_magic(IO *arc_io)
    {
        char magic[fjsys_magic_length];
        io_read_string(arc_io, magic, fjsys_magic_length);
        return memcmp(magic, fjsys_magic, fjsys_magic_length) == 0;
    }

    FjsysHeader *fjsys_read_header(IO *arc_io)
    {
        FjsysHeader *header = new FjsysHeader;
        assert(header != nullptr);
        header->header_size = io_read_u32_le(arc_io);
        header->file_names_size = io_read_u32_le(arc_io);
        header->file_count = io_read_u32_le(arc_io);
        io_skip(arc_io, 64);
        return header;
    }

    std::unique_ptr<VirtualFile> fjsys_read_file(void *context)
    {
        std::unique_ptr<VirtualFile> file(new VirtualFile);
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
        file->name = std::string(file_name);
        delete []file_name;

        io_seek(unpack_context->arc_io, data_offset);
        io_write_string_from_io(&file->io, unpack_context->arc_io, data_size);
        unpack_context->mgd_converter->try_decode(*file);

        io_seek(unpack_context->arc_io, old_pos);
        return file;
    }
}

struct FjsysArchive::Context
{
    MgdConverter *mgd_converter;
};

FjsysArchive::FjsysArchive()
{
    context = new FjsysArchive::Context();
    context->mgd_converter = new MgdConverter();
}

FjsysArchive::~FjsysArchive()
{
    delete context->mgd_converter;
    delete context;
}

void FjsysArchive::add_cli_help(ArgParser &arg_parser)
{
    context->mgd_converter->add_cli_help(arg_parser);
}

void FjsysArchive::parse_cli_options(ArgParser &arg_parser)
{
    context->mgd_converter->parse_cli_options(arg_parser);
}

bool FjsysArchive::unpack_internal(IO *arc_io, OutputFiles &output_files)
{
    assert(context != nullptr);
    if (!fjsys_check_magic(arc_io))
    {
        log_error("FJSYS: Not a FJSYS archive");
        return false;
    }

    FjsysHeader *header = fjsys_read_header(arc_io);
    assert(header != nullptr);

    FjsysUnpackContext unpack_context;
    unpack_context.header = header;
    unpack_context.mgd_converter = context->mgd_converter;
    unpack_context.arc_io = arc_io;

    size_t i;
    for (i = 0; i < header->file_count; i ++)
        output_files.save(&fjsys_read_file, &unpack_context);

    delete header;
    return true;
}
