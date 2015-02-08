// ARC archive
//
// Company:   -
// Engine:    BGI/Ethornell
// Extension: .arc
//
// Known games:
// - Higurashi No Naku Koro Ni

#include "formats/arc/arc_archive.h"
#include "formats/gfx/cbg_converter.h"

namespace
{
    const std::string magic("PackFile    ", 12);

    typedef struct UnpackContext
    {
        IO &arc_io;
        CbgConverter &cbg_converter;
        size_t file_count;

        UnpackContext(IO &arc_io, CbgConverter &cbg_converter)
            : arc_io(arc_io), cbg_converter(cbg_converter)
        {
        }
    } UnpackContext;

    std::unique_ptr<VirtualFile> read_file(void *context)
    {
        UnpackContext *unpack_context = (UnpackContext*)context;
        std::unique_ptr<VirtualFile> file(new VirtualFile);

        size_t old_pos = unpack_context->arc_io.tell();
        file->name = unpack_context->arc_io.read_until_zero();
        unpack_context->arc_io.seek(old_pos + 16);

        size_t offset = unpack_context->arc_io.read_u32_le();
        size_t size = unpack_context->arc_io.read_u32_le();
        offset += magic.size() + 4 + unpack_context->file_count * 32;
        unpack_context->arc_io.skip(8);
        if (offset + size > unpack_context->arc_io.size())
            throw std::runtime_error("Bad offset to file");

        old_pos = unpack_context->arc_io.tell();
        unpack_context->arc_io.seek(offset);
        file->io.write_from_io(unpack_context->arc_io, size);
        unpack_context->arc_io.seek(old_pos);

        unpack_context->cbg_converter.try_decode(*file);

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

void ArcArchive::unpack_internal(IO &arc_io, OutputFiles &output_files) const
{
    if (arc_io.read(magic.size()) != magic)
        throw std::runtime_error("Not an ARC archive");

    size_t file_count = arc_io.read_u32_le();
    if (file_count * 32 > arc_io.size())
        throw std::runtime_error("Bad file count");

    UnpackContext unpack_context(arc_io, *context->cbg_converter);
    unpack_context.file_count = file_count;
    size_t i;
    for (i = 0; i < file_count; i ++)
        output_files.save(&read_file, &unpack_context);
}
