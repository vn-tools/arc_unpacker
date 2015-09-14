#include "log.h"
#include "test_support/suppress_output.h"

using namespace au;

void tests::suppress_output(std::function<void()> callback)
{
    //don't log garbage from intermediate classes in tests...
    Log.mute();
    try
    {
        callback();
        Log.unmute();
    }
    catch (...)
    {
        //...but log the exceptions
        Log.unmute();
        throw;
    }
}
