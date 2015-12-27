#include "fmt/base_decoder.h"

using namespace au;
using namespace au::fmt;

void BaseDecoder::register_cli_options(ArgParser &) const
{
}

void BaseDecoder::parse_cli_options(const ArgParser &)
{
}

std::vector<std::string> BaseDecoder::get_linked_formats() const
{
    return {};
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
