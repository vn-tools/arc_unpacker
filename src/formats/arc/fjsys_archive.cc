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

    std::unique_ptr<Header> read_header(IO &arc_io)
    {
        std::unique_ptr<Header> header(new Header);
        header->header_size = arc_io.read_u32_le();
        header->file_names_size = arc_io.read_u32_le();
        header->file_count = arc_io.read_u32_le();
        arc_io.skip(64);
        return header;
    }

    std::unique_ptr<VirtualFile> read_file(
        IO &arc_io,
        const MgdConverter &mgd_converter,
        const Header &header)
    {
        std::unique_ptr<VirtualFile> file(new VirtualFile);
        size_t file_name_offset = arc_io.read_u32_le();
        size_t data_size = arc_io.read_u32_le();
        size_t data_offset = static_cast<size_t>(arc_io.read_u64_le());
        size_t old_pos = arc_io.tell();
        size_t file_names_start = header.header_size
            - header.file_names_size;

        arc_io.seek(file_name_offset + file_names_start);
        file->name = arc_io.read_until_zero();

        arc_io.seek(data_offset);
        file->io.write_from_io(arc_io, data_size);
        mgd_converter.try_decode(*file);

        arc_io.seek(old_pos);
        return file;
    }
}

struct FjsysArchive::Internals
{
    MgdConverter mgd_converter;
};

FjsysArchive::FjsysArchive() : internals(new Internals)
{
}

FjsysArchive::~FjsysArchive()
{
}

void FjsysArchive::add_cli_help(ArgParser &arg_parser) const
{
    internals->mgd_converter.add_cli_help(arg_parser);
}

void FjsysArchive::parse_cli_options(ArgParser &arg_parser)
{
    internals->mgd_converter.parse_cli_options(arg_parser);
}

void FjsysArchive::unpack_internal(
    VirtualFile &file, OutputFiles &output_files) const
{
    if (file.io.read(magic.size()) != magic)
        throw std::runtime_error("Not a FJSYS archive");

    std::unique_ptr<Header> header = read_header(file.io);

    for (size_t i = 0; i < header->file_count; i ++)
    {
        output_files.save([&]()
        {
            return read_file(file.io, internals->mgd_converter, *header);
        });
    }
}
