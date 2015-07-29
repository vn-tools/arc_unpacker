#include <iostream>
#include <cstdarg>
#include <memory>
#include "util/format.h"

std::string au::util::format(const std::string fmt, ...)
{
    va_list ap;
    size_t size;

    {
        const size_t BUFFER_SIZE = 256;
        char buffer[BUFFER_SIZE];
        va_start(ap, fmt);
        size = vsnprintf(buffer, BUFFER_SIZE, fmt.c_str(), ap);
        va_end(ap);
        if (size < BUFFER_SIZE)
            return std::string(buffer, size);
    }

    size++;
    std::unique_ptr<char[]> buf(new char[size]);

    va_start(ap, fmt);
    vsnprintf(buf.get(), size, fmt.c_str(), ap);
    va_end(ap);

    return std::string(buf.get(), buf.get() + size - 1);
}
