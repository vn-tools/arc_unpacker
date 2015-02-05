#include <cassert>
#include "formats/archive.h"
#include "logger.h"

Archive *archive_create()
{
    Archive *archive = new Archive;
    assert(archive != nullptr);
    archive->data = nullptr;
    archive->add_cli_help = nullptr;
    archive->parse_cli_options = nullptr;
    archive->unpack = nullptr;
    archive->cleanup = nullptr;
    return archive;
}

void archive_destroy(Archive *archive)
{
    assert(archive != nullptr);
    if (archive->cleanup != nullptr)
        archive->cleanup(archive);
    delete archive;
}

void archive_parse_cli_options(Archive *archive, ArgParser &arg_parser)
{
    assert(archive != nullptr);
    if (archive->parse_cli_options != nullptr)
        archive->parse_cli_options(archive, arg_parser);
}

void archive_add_cli_help(Archive *archive, ArgParser &arg_parser)
{
    assert(archive != nullptr);
    if (archive->add_cli_help != nullptr)
        archive->add_cli_help(archive, arg_parser);
}

bool archive_unpack(
    Archive *archive,
    IO *arc_io,
    OutputFiles *output_files)
{
    assert(archive != nullptr);
    if (archive->unpack == nullptr)
    {
        log_error("Unpacking for this format is not supported");
        return false;
    }
    return archive->unpack(archive, arc_io, output_files);
}
