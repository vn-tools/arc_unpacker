#include "bin_helpers.h"
#include "logger.h"

void add_help_option(ArgParser &arg_parser)
{
    arg_parser.add_help("-h, --help", "Shows this message.");
}

bool should_show_help(ArgParser &arg_parser)
{
    return arg_parser.has_flag("-h") || arg_parser.has_flag("--help");
}
