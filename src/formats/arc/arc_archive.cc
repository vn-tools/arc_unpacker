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

    std::unique_ptr<VirtualFile> read_file(
        IO &arc_io,
        const CbgConverter &cbg_converter,
        size_t file_count)
    {
        std::unique_ptr<VirtualFile> file(new VirtualFile);

        size_t old_pos = arc_io.tell();
        file->name = arc_io.read_until_zero();
        arc_io.seek(old_pos + 16);

        size_t offset = arc_io.read_u32_le();
        size_t size = arc_io.read_u32_le();
        offset += magic.size() + 4 + file_count * 32;
        arc_io.skip(8);
        if (offset + size > arc_io.size())
            throw std::runtime_error("Bad offset to file");

        old_pos = arc_io.tell();
        arc_io.seek(offset);
        file->io.write_from_io(arc_io, size);
        arc_io.seek(old_pos);

        cbg_converter.try_decode(*file);

        return file;
    }
}

struct ArcArchive::Internals
{
    CbgConverter cbg_converter;
};

ArcArchive::ArcArchive() : internals(new Internals)
{
}

ArcArchive::~ArcArchive()
{
}

void ArcArchive::add_cli_help(ArgParser &arg_parser) const
{
    internals->cbg_converter.add_cli_help(arg_parser);
}

void ArcArchive::parse_cli_options(ArgParser &arg_parser)
{
    internals->cbg_converter.parse_cli_options(arg_parser);
}

void ArcArchive::unpack_internal(
    VirtualFile &file, OutputFiles &output_files) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not an ARC archive");

    size_t file_count = file.io.read_u32_le();
    if (file_count * 32 > file.io.size())
        throw std::runtime_error("Bad file count");

    for (size_t i = 0; i < file_count; i ++)
    {
        output_files.save([&]()
        {
            return read_file(file.io, internals->cbg_converter, file_count);
        });
    }
}
