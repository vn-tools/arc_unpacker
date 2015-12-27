#include "fmt/idecoder.h"

using namespace au;
using namespace au::fmt;

io::path fmt::decorate_path(
    const IDecoder::NamingStrategy strategy,
    const io::path &parent_path,
    const io::path &current_path)
{
    if (strategy == IDecoder::NamingStrategy::Root)
        return current_path;

    if (strategy == IDecoder::NamingStrategy::Child)
    {
        if (parent_path.str() == "")
            return current_path;
        return parent_path / current_path;
    }

    if (strategy == IDecoder::NamingStrategy::Sibling)
    {
        if (parent_path.str() == "")
            return current_path;
        return parent_path.parent() / current_path;
    }

    if (strategy == IDecoder::NamingStrategy::FlatSibling)
    {
        if (parent_path.str() == "")
            return current_path.name();
        return parent_path.parent() / current_path.name();
    }

    throw std::logic_error("Invalid naming strategy");
}

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
