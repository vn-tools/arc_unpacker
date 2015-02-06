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

namespace
{
    const char *arc_magic = "PackFile    ";
    const size_t arc_magic_length = 12;

    typedef struct
    {
        CbgConverter *cbg_converter;
        IO *arc_io;
        size_t file_count;
    } ArcUnpackContext;

    bool arc_check_magic(IO *arc_io)
    {
        char magic[arc_magic_length];
        io_read_string(arc_io, magic, arc_magic_length);
        return memcmp(magic, arc_magic, arc_magic_length) == 0;
    }

    std::unique_ptr<VirtualFile> arc_read_file(void *context)
    {
        ArcUnpackContext *unpack_context = (ArcUnpackContext*)context;
        assert(unpack_context != nullptr);

        std::unique_ptr<VirtualFile> file(new VirtualFile);

        size_t old_pos = io_tell(unpack_context->arc_io);
        char *tmp_name;
        io_read_until_zero(unpack_context->arc_io, &tmp_name, nullptr);
        assert(tmp_name != nullptr);
        file->name = std::string(tmp_name);
        delete []tmp_name;
        io_seek(unpack_context->arc_io, old_pos + 16);

        size_t offset = io_read_u32_le(unpack_context->arc_io);
        size_t size = io_read_u32_le(unpack_context->arc_io);
        offset += arc_magic_length + 4 + unpack_context->file_count * 32;
        io_skip(unpack_context->arc_io, 8);
        if (offset + size > io_size(unpack_context->arc_io))
        {
            log_error("ARC: Bad offset to file");
            return nullptr;
        }

        old_pos = io_tell(unpack_context->arc_io);
        if (!io_seek(unpack_context->arc_io, offset))
        {
            log_error("ARC: Failed to seek to file");
            return nullptr;
        }
        io_write_string_from_io(&file->io, unpack_context->arc_io, size);
        io_seek(unpack_context->arc_io, old_pos);

        unpack_context->cbg_converter->try_decode(*file);

        return file;
    }
}

struct ArcArchive::Context
{
    CbgConverter *cbg_converter;
};

ArcArchive::ArcArchive()
{
    context = new ArcArchive::Context();
    context->cbg_converter = new CbgConverter();
}

ArcArchive::~ArcArchive()
{
    delete context->cbg_converter;
    delete context;
}

void ArcArchive::add_cli_help(ArgParser &arg_parser)
{
    context->cbg_converter->add_cli_help(arg_parser);
}

void ArcArchive::parse_cli_options(ArgParser &arg_parser)
{
    context->cbg_converter->parse_cli_options(arg_parser);
}

bool ArcArchive::unpack_internal(IO *arc_io, OutputFiles &output_files)
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
    unpack_context.cbg_converter = context->cbg_converter;
    unpack_context.arc_io = arc_io;
    unpack_context.file_count = file_count;
    size_t i;
    for (i = 0; i < file_count; i ++)
    {
        output_files.save(&arc_read_file, &unpack_context);
    }
    return true;
}
