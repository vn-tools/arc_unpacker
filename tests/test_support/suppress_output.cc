#include <iostream>
#include "test_support/suppress_output.h"

void suppress_output(std::function<void()> callback)
{
    auto *old_cout_buf = std::cout.rdbuf();
    //don't log garbage from intermediate classes in tests...
    std::cout.rdbuf(nullptr);
    try
    {
        callback();
        std::cout.rdbuf(old_cout_buf);
    }
    catch (...)
    {
        //...but log the exceptions
        std::cout.rdbuf(old_cout_buf);
        throw;
    }
}
