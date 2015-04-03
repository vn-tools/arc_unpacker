#include "formats/archive.h"

void Archive::add_cli_help(ArgParser &arg_parser) const
{
    Transformer::add_cli_help(arg_parser);
}

void Archive::parse_cli_options(const ArgParser &arg_parser)
{
    Transformer::parse_cli_options(arg_parser);
}

FileNamingStrategy Archive::get_file_naming_strategy() const
{
    return FileNamingStrategy::Child;
}

Archive::~Archive()
{
}
