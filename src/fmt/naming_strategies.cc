#include "fmt/naming_strategies.h"
#include "io/path.h"

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
    auto path = io::path(parent_name);
    path = path.parent();
    path /= current_name;
    return path.str();
}

std::string ChildNamingStrategy::decorate(
    const std::string &parent_name,
    const std::string &current_name) const
{
    if (parent_name == "")
        return current_name;
    auto path = io::path(parent_name);
    path /= current_name;
    return path.str();
}
