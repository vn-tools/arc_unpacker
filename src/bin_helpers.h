#ifndef BIN_HELPERS_H
#define BIN_HELPERS_H
#include "arg_parser.h"

void add_quiet_option(ArgParser &arg_parser);
void add_help_option(ArgParser &arg_parser);
bool should_show_help(ArgParser &arg_parser);

#endif
