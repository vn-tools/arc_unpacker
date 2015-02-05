#include <cassert>
#include "formats/archive.h"
#include "logger.h"

Archive *archive_create()
{
    Archive *archive = new Archive;
    assert(archive != NULL);
    archive->data = NULL;
    archive->add_cli_help = NULL;
    archive->parse_cli_options = NULL;
    archive->unpack = NULL;
    archive->cleanup = NULL;
    return archive;
}

void archive_destroy(Archive *archive)
{
    assert(archive != NULL);
    if (archive->cleanup != NULL)
        archive->cleanup(archive);
    delete archive;
}

void archive_parse_cli_options(Archive *archive, ArgParser *arg_parser)
{
    assert(archive != NULL);
    if (archive->parse_cli_options != NULL)
        archive->parse_cli_options(archive, arg_parser);
}

void archive_add_cli_help(Archive *archive, ArgParser *arg_parser)
{
    assert(archive != NULL);
    if (archive->add_cli_help != NULL)
        archive->add_cli_help(archive, arg_parser);
}

bool archive_unpack(
    Archive *archive,
    IO *arc_io,
    OutputFiles *output_files)
{
    assert(archive != NULL);
    if (archive->unpack == NULL)
    {
        log_error("Unpacking for this format is not supported");
        return false;
    }
    return archive->unpack(archive, arc_io, output_files);
}
