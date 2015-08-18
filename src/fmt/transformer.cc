#include <boost/filesystem/path.hpp>
#include <stdexcept>
#include "fmt/transformer.h"

using namespace au;
using namespace au::fmt;

std::string FileNameDecorator::decorate(
    const FileNamingStrategy &strategy,
    const std::string &parent_file_name,
    const std::string &current_file_name)
{
    switch (strategy)
    {
        case FileNamingStrategy::Root:
            return current_file_name;

        case FileNamingStrategy::Child:
        case FileNamingStrategy::Sibling:
        {
            if (parent_file_name == "")
                return current_file_name;
            auto path = boost::filesystem::path(parent_file_name);
            if (strategy == FileNamingStrategy::Sibling)
                path = path.parent_path();
            path /= current_file_name;
            return path.string();
        }

        default:
            throw std::runtime_error("Invalid file naming strategy");
    }
}

bool Transformer::is_recognized(File &file) const
{
    try
    {
        file.io.seek(0);
        return is_recognized_internal(file);
    }
    catch (...)
    {
        return false;
    }
}

Transformer::~Transformer()
{
}
