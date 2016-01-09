#include "arg_parser_decorator.h"

using namespace au;

ArgParserDecorator::ArgParserDecorator(
    const std::function<void(ArgParser &)> register_callback,
    const std::function<void(const ArgParser &)> parse_callback)
    : register_callback(register_callback), parse_callback(parse_callback)
{
}

void ArgParserDecorator::register_cli_options(ArgParser &arg_parser) const
{
    register_callback(arg_parser);
}

void ArgParserDecorator::parse_cli_options(const ArgParser &arg_parser) const
{
    parse_callback(arg_parser);
}
