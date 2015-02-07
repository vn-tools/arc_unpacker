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
#include <memory>
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

    typedef struct FjsysUnpackContext
    {
        IO &arc_io;
        MgdConverter &mgd_converter;
        FjsysHeader &header;

        FjsysUnpackContext(
            IO &arc_io, MgdConverter &mgd_converter, FjsysHeader &header)
            : arc_io(arc_io), mgd_converter(mgd_converter), header(header)
        {
        }
    } FjsysUnpackContext;

    bool fjsys_check_magic(IO &arc_io)
    {
        char magic[fjsys_magic_length];
        arc_io.read(magic, fjsys_magic_length);
        return memcmp(magic, fjsys_magic, fjsys_magic_length) == 0;
    }

    std::unique_ptr<FjsysHeader> fjsys_read_header(IO &arc_io)
    {
        std::unique_ptr<FjsysHeader> header(new FjsysHeader);
        header->header_size = arc_io.read_u32_le();
        header->file_names_size = arc_io.read_u32_le();
        header->file_count = arc_io.read_u32_le();
        arc_io.skip(64);
        return header;
    }

    std::unique_ptr<VirtualFile> fjsys_read_file(void *context)
    {
        std::unique_ptr<VirtualFile> file(new VirtualFile);
        FjsysUnpackContext *unpack_context = (FjsysUnpackContext*)context;
        size_t file_name_offset = unpack_context->arc_io.read_u32_le();
        size_t data_size = unpack_context->arc_io.read_u32_le();
        size_t data_offset = unpack_context->arc_io.read_u64_le();
        size_t old_pos = unpack_context->arc_io.tell();
        size_t file_names_start = unpack_context->header.header_size
            - unpack_context->header.file_names_size;

        unpack_context->arc_io.seek(file_name_offset + file_names_start);
        char *file_name = nullptr;
        unpack_context->arc_io.read_until_zero(&file_name, nullptr);
        assert(file_name != nullptr);
        file->name = std::string(file_name);
        delete []file_name;

        unpack_context->arc_io.seek(data_offset);
        file->io.write_from_io(unpack_context->arc_io, data_size);
        unpack_context->mgd_converter.try_decode(*file);

        unpack_context->arc_io.seek(old_pos);
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

bool FjsysArchive::unpack_internal(IO &arc_io, OutputFiles &output_files)
{
    assert(context != nullptr);
    if (!fjsys_check_magic(arc_io))
    {
        log_error("FJSYS: Not a FJSYS archive");
        return false;
    }

    std::unique_ptr<FjsysHeader> header = fjsys_read_header(arc_io);
    assert(header != nullptr);

    FjsysUnpackContext unpack_context(
        arc_io, *context->mgd_converter, *header);
    size_t i;
    for (i = 0; i < header->file_count; i ++)
        output_files.save(&fjsys_read_file, &unpack_context);

    return true;
}
