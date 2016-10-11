// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

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
