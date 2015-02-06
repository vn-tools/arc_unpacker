// MBL archive
//
// Company:   Ivory
// Engine:    MarbleEngine
// Extension: .mbl
//
// Known games:
// - Wanko to Kurasou

#include <cassert>
#include <cstring>
#include "formats/arc/mbl_archive.h"
#include "formats/gfx/prs_converter.h"
#include "logger.h"
#include "string_ex.h"

namespace
{
    typedef struct
    {
        PrsConverter *prs_converter;
        IO *arc_io;
        size_t name_length;
    } MblUnpackContext;

    int mbl_check_version(
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

    int mbl_get_version(IO *arc_io)
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

    VirtualFile *mbl_read_file(void *context)
    {
        MblUnpackContext *unpack_context = (MblUnpackContext*)context;
        assert(unpack_context != nullptr);
        VirtualFile *file = virtual_file_create();

        size_t old_pos = io_tell(unpack_context->arc_io);
        char *tmp_name = nullptr;
        io_read_until_zero(unpack_context->arc_io, &tmp_name, nullptr);
        assert(tmp_name != nullptr);

        char *decoded_name;
        if (!convert_encoding(
            tmp_name, strlen(tmp_name),
            &decoded_name, nullptr,
            "sjis", "utf-8"))
        {
            assert(0);
        }

        virtual_file_set_name(file, decoded_name);
        delete []decoded_name;

        delete []tmp_name;
        io_seek(unpack_context->arc_io, old_pos + unpack_context->name_length);

        size_t offset = io_read_u32_le(unpack_context->arc_io);
        size_t size = io_read_u32_le(unpack_context->arc_io);
        if (offset + size > io_size(unpack_context->arc_io))
        {
            log_error("MBL: Bad offset to file");
            return false;
        }

        old_pos = io_tell(unpack_context->arc_io);
        io_seek(unpack_context->arc_io, offset);
        io_write_string_from_io(file->io, unpack_context->arc_io, size);
        io_seek(unpack_context->arc_io, old_pos);

        unpack_context->prs_converter->try_decode(file);

        return file;
    }
}

struct MblArchive::Context
{
    PrsConverter *prs_converter;
};

MblArchive::MblArchive()
{
    context = new MblArchive::Context();
    context->prs_converter = new PrsConverter();
}

MblArchive::~MblArchive()
{
    delete context->prs_converter;
    delete context;
}

void MblArchive::add_cli_help(ArgParser &arg_parser)
{
    context->prs_converter->add_cli_help(arg_parser);
}

void MblArchive::parse_cli_options(ArgParser &arg_parser)
{
    context->prs_converter->parse_cli_options(arg_parser);
}

bool MblArchive::unpack_internal(IO *arc_io, OutputFiles &output_files)
{
    size_t i;
    int version = mbl_get_version(arc_io);
    if (version == -1)
    {
        log_error("MBL: Not a MBL archive");
        return false;
    }
    log_info("MBL: Version: %d", version);

    uint32_t file_count = io_read_u32_le(arc_io);
    uint32_t name_length = version == 2 ? io_read_u32_le(arc_io) : 16;
    MblUnpackContext unpack_context;
    unpack_context.prs_converter = context->prs_converter;
    unpack_context.arc_io = arc_io;
    unpack_context.name_length = name_length;
    for (i = 0; i < file_count; i ++)
    {
        output_files.save(&mbl_read_file, &unpack_context);
    }

    return true;
}
