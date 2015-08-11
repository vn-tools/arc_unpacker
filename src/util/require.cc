#include "util/format.h"
#include "util/require.h"

using namespace au;

std::exception util::fail_impl(
    const std::string &message,
    const std::string &func,
    const std::string &file,
    int line)
{
    return std::runtime_error(util::format(
        "Assertion failed in %s() at %s:%d: %s",
        func.c_str(),
        file.c_str(),
        line,
        message.c_str()));
}

void util::require_impl(
    bool condition,
    const std::string &expression,
    const std::string &func,
    const std::string &file,
    int line)
{
    if (!condition)
    {
        throw std::runtime_error(util::format(
            "Assertion \"%s\" failed in %s() at %s:%d",
            expression.c_str(),
            func.c_str(),
            file.c_str(),
            line));
    }
}
