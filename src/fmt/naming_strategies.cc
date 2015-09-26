#include "fmt/naming_strategies.h"
#include <boost/filesystem/path.hpp>

using namespace au;
using namespace au::fmt;

std::string RootNamingStrategy::decorate(
    const std::string &parent_name,
    const std::string &current_name) const
{
    return current_name;
}

std::string SiblingNamingStrategy::decorate(
    const std::string &parent_name,
    const std::string &current_name) const
{
    if (parent_name == "")
        return current_name;
    auto path = boost::filesystem::path(parent_name);
    path = path.parent_path();
    path /= current_name;
    return path.string();
}

std::string ChildNamingStrategy::decorate(
    const std::string &parent_name,
    const std::string &current_name) const
{
    if (parent_name == "")
        return current_name;
    auto path = boost::filesystem::path(parent_name);
    path /= current_name;
    return path.string();
}
