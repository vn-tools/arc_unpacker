#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "assert.h"
#include "logger.h"

static char buffer[1024];
static bool levels_enabled[_LOG_LEVEL_COUNT];

static void log_text(FILE *out, const char *prefix, const char *buffer);

static void log_text(FILE *out, const char *prefix, const char *buffer)
{
    if (errno != 0)
        fprintf(out, "%s: %s (%s)\n", prefix, buffer, strerror(errno));
    else
        fprintf(out, "%s: %s\n", prefix, buffer);
}



bool log_enabled(LogLevel level)
{
    assert_that(level < _LOG_LEVEL_COUNT);
    return levels_enabled[level];
}

void log_enable(LogLevel level)
{
    assert_that(level < _LOG_LEVEL_COUNT);
    levels_enabled[level] = true;
}

void log_disable(LogLevel level)
{
    assert_that(level < _LOG_LEVEL_COUNT);
    levels_enabled[level] = false;
}

void log_error(char *format, ...)
{
    va_list args;
    if (!levels_enabled[LOG_LEVEL_ERROR])
        return;

    va_start(args, format);
    vsnprintf(buffer, sizeof buffer, format, args);
    va_end(args);

    log_text(stderr, "Error", buffer);
}

void log_warning(char *format, ...)
{
    va_list args;
    if (!levels_enabled[LOG_LEVEL_WARNING])
        return;

    va_start(args, format);
    vsnprintf(buffer, sizeof buffer, format, args);
    va_end(args);

    log_text(stderr, "Warning", buffer);
}

void log_info(char *format, ...)
{
    va_list args;
    if (!levels_enabled[LOG_LEVEL_INFO])
        return;

    va_start(args, format);
    vsnprintf(buffer, sizeof buffer, format, args);
    va_end(args);

    log_text(stdout, "Info", buffer);
}
