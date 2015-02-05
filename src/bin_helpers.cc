#include <cassert>
#include "bin_helpers.h"
#include "logger.h"

void add_quiet_option(ArgParser &arg_parser)
{
    arg_parser.add_help("-q, --quiet", "Suppresses output.");

    log_enable(LOG_LEVEL_WARNING);
    log_enable(LOG_LEVEL_ERROR);

    if (arg_parser.has_flag("-q") || arg_parser.has_flag("--quiet"))
        log_disable(LOG_LEVEL_INFO);
    else
        log_enable(LOG_LEVEL_INFO);
}

void add_help_option(ArgParser &arg_parser)
{
    arg_parser.add_help("-h, --help", "Shows this message.");
}

bool should_show_help(ArgParser &arg_parser)
{
    return arg_parser.has_flag("-h") || arg_parser.has_flag("--help");
}
