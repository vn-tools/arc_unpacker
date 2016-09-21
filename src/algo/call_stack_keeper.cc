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
