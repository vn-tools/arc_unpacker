#include "algo/format.h"
#include <cstdio>
#include <memory>

using namespace au;

std::string algo::format(const std::string fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    const auto ret = format(fmt, args);
    va_end(args);
    return ret;
}

std::string algo::format(const std::string fmt, std::va_list args)
{
    size_t size;
    std::va_list args_copy;
    va_copy(args_copy, args);

    {
        const size_t buffer_size = 256;
        char buffer[buffer_size];
        size = vsnprintf(buffer, buffer_size, fmt.c_str(), args);
        if (size < buffer_size)
        {
            va_end(args_copy);
            return std::string(buffer, size);
        }
    }

    size++;
    auto buf = std::make_unique<char[]>(size);
    vsnprintf(buf.get(), size, fmt.c_str(), args_copy);
    va_end(args_copy);
    return std::string(buf.get(), buf.get() + size - 1);
}
