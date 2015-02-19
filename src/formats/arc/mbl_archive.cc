// MBL archive
//
// Company:   Ivory
// Engine:    MarbleEngine
// Extension: .mbl
//
// Known games:
// - Wanko to Kurasou

#include "formats/arc/mbl_archive.h"
#include "formats/gfx/prs_converter.h"
#include "string/encoding.h"

namespace
{
    int check_version(
        IO &arc_io,
        size_t initial_position,
        uint32_t file_count,
        uint32_t name_length)
    {
        arc_io.seek(initial_position + file_count * (name_length + 8));
        arc_io.skip(-8);
        uint32_t last_file_offset = arc_io.read_u32_le();
        uint32_t last_file_size = arc_io.read_u32_le();
        return last_file_offset + last_file_size == arc_io.size();
    }

    int get_version(IO &arc_io)
    {
        uint32_t file_count = arc_io.read_u32_le();
        if (check_version(arc_io, 4, file_count, 16))
        {
            arc_io.seek(0);
            return 1;
        }

        arc_io.seek(4);
        uint32_t name_length = arc_io.read_u32_le();
        if (check_version(arc_io, 8, file_count, name_length))
        {
            arc_io.seek(0);
            return 2;
        }

        return -1;
    }

    std::unique_ptr<File> read_file(
        IO &arc_io,
        const PrsConverter &prs_converter,
        size_t name_length)
    {
        std::unique_ptr<File> file(new File);

        size_t old_pos = arc_io.tell();
        std::string name = arc_io.read_until_zero();
        file->name = convert_encoding(name, "sjis", "utf-8");
        arc_io.seek(old_pos + name_length);

        size_t offset = arc_io.read_u32_le();
        size_t size = arc_io.read_u32_le();
        if (offset + size > arc_io.size())
            throw std::runtime_error("Bad offset to file");

        old_pos = arc_io.tell();
        arc_io.seek(offset);
        file->io.write_from_io(arc_io, size);
        arc_io.seek(old_pos);

        prs_converter.try_decode(*file);
        return file;
    }
}

struct MblArchive::Internals
{
    PrsConverter prs_converter;
};

MblArchive::MblArchive() : internals(new Internals)
{
}

MblArchive::~MblArchive()
{
}

void MblArchive::add_cli_help(ArgParser &arg_parser) const
{
    internals->prs_converter.add_cli_help(arg_parser);
}

void MblArchive::parse_cli_options(ArgParser &arg_parser)
{
    internals->prs_converter.parse_cli_options(arg_parser);
}

void MblArchive::unpack_internal(File &file, FileSaver &file_saver) const
{
    int version = get_version(file.io);
    if (version == -1)
        throw std::runtime_error("Not a MBL archive");

    uint32_t file_count = file.io.read_u32_le();
    uint32_t name_length = version == 2 ? file.io.read_u32_le() : 16;

    for (size_t i = 0; i < file_count; i ++)
    {
        file_saver.save(
            read_file(file.io, internals->prs_converter, name_length));
    }
}
