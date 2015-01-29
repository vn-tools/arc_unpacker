#include <stdlib.h>
#include "assert.h"
#include "logger.h"
#include "cli_helpers.h"

void cli_add_quiet_option(ArgParser *arg_parser)
{
    assert_not_null(arg_parser);
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

void cli_add_help_option(ArgParser *arg_parser)
{
    assert_not_null(arg_parser);
    arg_parser_add_help(arg_parser, "-h, --help", "Shows this message.");
}

bool cli_should_show_help(ArgParser *arg_parser)
{
    assert_not_null(arg_parser);
    return arg_parser_has_flag(arg_parser, "-h")
        || arg_parser_has_flag(arg_parser, "--help");
}
