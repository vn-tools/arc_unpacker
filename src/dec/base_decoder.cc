#include "dec/base_decoder.h"

using namespace au;
using namespace au::dec;

std::vector<ArgParserDecorator> BaseDecoder::get_arg_parser_decorators() const
{
    return arg_parser_decorators;
}

std::vector<std::string> BaseDecoder::get_linked_formats() const
{
    return {};
}

void BaseDecoder::add_arg_parser_decorator(const ArgParserDecorator &decorator)
{
    arg_parser_decorators.push_back(decorator);
}

void BaseDecoder::add_arg_parser_decorator(
    const std::function<void(ArgParser &)> register_callback,
    const std::function<void(const ArgParser &)> parse_callback)
{
    const ArgParserDecorator decorator(register_callback, parse_callback);
    add_arg_parser_decorator(decorator);
}

bool BaseDecoder::is_recognized(io::File &input_file) const
{
    try
    {
        input_file.stream.seek(0);
        return is_recognized_impl(input_file);
    }
    catch (...)
    {
        return false;
    }
}
