#include <iostream>
#include <cstdio>
#include <cstdarg>
#include <memory>
#include "util/format.h"

std::string au::util::format(const std::string fmt, ...)
{
    va_list ap;
    size_t size;

    {
        const size_t buffer_size = 256;
        char buffer[buffer_size];
        va_start(ap, fmt);
        size = vsnprintf(buffer, buffer_size, fmt.c_str(), ap);
        va_end(ap);
        if (size < buffer_size)
            return std::string(buffer, size);
    }

    size++;
    std::unique_ptr<char[]> buf(new char[size]);

    va_start(ap, fmt);
    vsnprintf(buf.get(), size, fmt.c_str(), ap);
    va_end(ap);

    return std::string(buf.get(), buf.get() + size - 1);
}
