#include <assert.h>
#include <stdlib.h>
#include "bin_helpers.h"
#include "logger.h"

void add_quiet_option(ArgParser *arg_parser)
{
    assert(arg_parser != NULL);
    arg_parser_add_help(
        arg_parser,
        "-q, --quiet",
        "Suppresses output.");

    log_enable(LOG_LEVEL_WARNING);
    log_enable(LOG_LEVEL_ERROR);

    if (arg_parser_has_flag(arg_parser, "-q")
    || arg_parser_has_flag(arg_parser, "--quiet"))
    {
        log_disable(LOG_LEVEL_INFO);
    }
    else
    {
        log_enable(LOG_LEVEL_INFO);
    }
}

void add_help_option(ArgParser *arg_parser)
{
    assert(arg_parser != NULL);
    arg_parser_add_help(arg_parser, "-h, --help", "Shows this message.");
}

bool should_show_help(ArgParser *arg_parser)
{
    assert(arg_parser != NULL);
    return arg_parser_has_flag(arg_parser, "-h")
        || arg_parser_has_flag(arg_parser, "--help");
}
