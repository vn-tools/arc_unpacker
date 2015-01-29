#ifndef CLI_H
#define CLI_H
#include "arg_parser.h"

void cli_add_quiet_option(ArgParser *arg_parser);
void cli_add_help_option(ArgParser *arg_parser);
bool cli_should_show_help(ArgParser *arg_parser);

#endif
