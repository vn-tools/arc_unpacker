#include <errno.h>
#include "formats/arc/sar_archive.h"
#include "assert.h"
#include "logger.h"

static bool unpack(Archive *archive, IO *archive_file, OutputFiles *output_files);

static void add_cli_help(Archive *archive, ArgParser *arg_parser)
{
    assert_not_null(archive);
    assert_not_null(arg_parser);
    log_info("Adding help...");
}

static void parse_cli_options(Archive *archive, ArgParser *arg_parser)
{
    assert_not_null(archive);
    assert_not_null(arg_parser);
    log_info("Parsing CLI options...");
}

static void cleanup(Archive *archive)
{
    assert_not_null(archive);
    log_info("Cleanup...");
}

static bool unpack(Archive *archive, IO *archive_file, OutputFiles *output_files)
{
    assert_not_null(archive);
    assert_not_null(archive_file);
    assert_not_null(output_files);
    log_info("Unpacking...");
    return true;
}

Archive *sar_archive_create()
{
    Archive *archive = archive_create();
    archive->add_cli_help = &add_cli_help;
    archive->parse_cli_options = &parse_cli_options;
    archive->unpack = &unpack;
    archive->cleanup = &cleanup;
    return archive;
}
