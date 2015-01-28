#include <stdlib.h>
#include "assert.h"
#include "cli_helpers.h"

void cli_add_quiet_option(ArgParser *arg_parser, Options *options)
{
    assert_not_null(arg_parser);
    arg_parser_add_help(
        arg_parser,
        "-q, --quiet",
        "Suppresses output.");

    if (arg_parser_has_flag(arg_parser, "-q")
        || arg_parser_has_flag(arg_parser, "--quiet"))
    {
        options_set(options, "verbosity", "quiet");
    }
}

void cli_add_verbose_option(ArgParser *arg_parser, Options *options)
{
    assert_not_null(arg_parser);
    arg_parser_add_help(
        arg_parser,
        "-v, --verbose",
        "Shows additional debug information>");

    options_set(options, "verbosity", "normal");
    if (arg_parser_has_flag(arg_parser, "-v")
        || arg_parser_has_flag(arg_parser, "--verbose"))
    {
        options_set(options, "verbosity", "verbose");
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
