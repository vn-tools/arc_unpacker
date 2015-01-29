#ifndef CLI_H
#define CLI_H
#include "arg_parser.h"
#include "options.h"

void cli_add_quiet_option(ArgParser *arg_parser, Options *options);
void cli_add_help_option(ArgParser *arg_parser);
bool cli_should_show_help(ArgParser *arg_parser);

#endif
