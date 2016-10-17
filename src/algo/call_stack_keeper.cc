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

#include "algo/call_stack_keeper.h"
#include "err.h"
#include <string>

using namespace au::algo;

struct CallStackKeeper::Priv final
{
    Priv(const size_t limit);
    size_t depth;
    size_t limit;
};

CallStackKeeper::Priv::Priv(const size_t limit) : depth(0), limit(limit)
{
}

CallStackKeeper::CallStackKeeper(const size_t limit) : p(new Priv(limit))
{
}

CallStackKeeper::~CallStackKeeper()
{
}

bool CallStackKeeper::recursion_limit_reached() const
{
    return p->depth >= p->limit;
}

void CallStackKeeper::recurse(const std::function<void()> &action)
{
    const std::string recursion_error = std::string("Recursion limit reached");
    if (recursion_limit_reached())
        throw err::GeneralError(recursion_error);
    p->depth++;
    try
    {
        action();
    }
    catch (...)
    {
        p->depth--;
        throw;
    }
    p->depth--;
}
