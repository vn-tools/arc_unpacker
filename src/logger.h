#ifndef LOG_H
#define LOG_H
#include <stdbool.h>

typedef enum
{
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    _LOG_LEVEL_COUNT,
} LogLevel;

bool log_enabled(LogLevel level);
void log_enable(LogLevel level);
void log_disable(LogLevel level);

void log_error(char *format, ...);
void log_warning(char *format, ...);
void log_info(char *format, ...);

#endif
