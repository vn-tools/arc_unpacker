#include "util/call_stack_keeper.h"

using namespace au::util;

static int call_depth = 0;

CallStackKeeper::CallStackKeeper()
{
    call_depth++;
}

CallStackKeeper::~CallStackKeeper()
{
    call_depth--;
}

int CallStackKeeper::current_call_depth() const
{
    return call_depth;
}
