#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "logger.h"

void log(const char *format, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsnprintf(buffer, sizeof buffer, format, args);
    va_end(args);

    printf("%s", buffer);
    fflush(stdout);
}
