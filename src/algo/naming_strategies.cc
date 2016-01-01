#include "algo/naming_strategies.h"
#include <stdexcept>

using namespace au;
using namespace au::algo;

io::path algo::apply_naming_strategy(
    const NamingStrategy strategy,
    const io::path &parent_path,
    const io::path &current_path)
{
    if (strategy == NamingStrategy::Root)
        return current_path;

    if (strategy == NamingStrategy::Child)
    {
        if (parent_path.str() == "")
            return current_path;
        return parent_path / current_path;
    }

    if (strategy == NamingStrategy::Sibling)
    {
        if (parent_path.str() == "")
            return current_path;
        return parent_path.parent() / current_path;
    }

    if (strategy == NamingStrategy::FlatSibling)
    {
        if (parent_path.str() == "")
            return current_path.name();
        return parent_path.parent() / current_path.name();
    }

    throw std::logic_error("Invalid naming strategy");
}
