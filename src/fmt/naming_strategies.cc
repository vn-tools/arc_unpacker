#include "fmt/naming_strategies.h"

using namespace au;
using namespace au::fmt;

io::path RootNamingStrategy::decorate(
    const io::path &parent_name,
    const io::path &current_name) const
{
    return current_name;
}

io::path SiblingNamingStrategy::decorate(
    const io::path &parent_name,
    const io::path &current_name) const
{
    if (parent_name.str() == "")
        return current_name;
    return parent_name.parent() / current_name;
}

io::path ChildNamingStrategy::decorate(
    const io::path &parent_name,
    const io::path &current_name) const
{
    if (parent_name.str() == "")
        return current_name;
    return parent_name / current_name;
}
