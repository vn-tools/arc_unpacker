// FJSYS archive
//
// Company:   various
// Engine:    -
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

#include "formats/arc/fjsys_archive.h"
#include "formats/gfx/mgd_converter.h"

namespace
{
    const std::string magic("FJSYS\x00\x00\x00", 8);

    typedef struct
    {
        size_t header_size;
        size_t file_names_size;
        size_t file_count;
    } Header;

    typedef struct UnpackContext
    {
        IO &arc_io;
        MgdConverter &mgd_converter;
        Header &header;

        UnpackContext(
            IO &arc_io, MgdConverter &mgd_converter, Header &header)
            : arc_io(arc_io), mgd_converter(mgd_converter), header(header)
        {
        }
    } UnpackContext;

    std::unique_ptr<Header> read_header(IO &arc_io)
    {
        std::unique_ptr<Header> header(new Header);
        header->header_size = arc_io.read_u32_le();
        header->file_names_size = arc_io.read_u32_le();
        header->file_count = arc_io.read_u32_le();
        arc_io.skip(64);
        return header;
    }

    std::unique_ptr<VirtualFile> read_file(void *context)
    {
        std::unique_ptr<VirtualFile> file(new VirtualFile);
        UnpackContext *unpack_context = (UnpackContext*)context;
        size_t file_name_offset = unpack_context->arc_io.read_u32_le();
        size_t data_size = unpack_context->arc_io.read_u32_le();
        size_t data_offset = unpack_context->arc_io.read_u64_le();
        size_t old_pos = unpack_context->arc_io.tell();
        size_t file_names_start = unpack_context->header.header_size
            - unpack_context->header.file_names_size;

        unpack_context->arc_io.seek(file_name_offset + file_names_start);
        file->name = unpack_context->arc_io.read_until_zero();

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

void FjsysArchive::unpack_internal(IO &arc_io, OutputFiles &output_files) const
{
    if (arc_io.read(magic.size()) != magic)
        throw std::runtime_error("Not a FJSYS archive");

    std::unique_ptr<Header> header = read_header(arc_io);

    UnpackContext unpack_context(arc_io, *context->mgd_converter, *header);
    size_t i;
    for (i = 0; i < header->file_count; i ++)
        output_files.save(&read_file, &unpack_context);
}
