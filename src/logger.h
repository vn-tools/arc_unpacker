#ifndef LOGGER_H
#define LOGGER_H
#include <stdbool.h>

typedef enum
{
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    _LOG_LEVEL_COUNT
} LogLevel;

void log_save();
void log_silence();
void log_restore();

bool log_enabled(LogLevel level);
void log_enable(LogLevel level);
void log_disable(LogLevel level);

void log_error(const char *format, ...);
void log_warning(const char *format, ...);
void log_info(const char *format, ...);

#endif
