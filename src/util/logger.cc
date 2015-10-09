#include "util/logger.h"
#include <cstdarg>
#include <iostream>
#include "util/format.h"

using namespace au::util;

struct Logger::Priv final
{
    Priv(Logger &logger);
    void log(MessageType type, std::string fmt, std::va_list args);

    Logger &logger;
    Color colors[5];
    int muted = 0;
    bool colors_enabled = true;
};

Logger::Priv::Priv(Logger &logger) : logger(logger)
{
    colors[MessageType::Info] = Color::Original;
    colors[MessageType::Success] = Color::Lime;
    colors[MessageType::Warning] = Color::Yellow;
    colors[MessageType::Error] = Color::Red;
    colors[MessageType::Debug] = Color::Magenta;
}

void Logger::Priv::log(
    MessageType type, std::string fmt, std::va_list args)
{
    if (muted & (1 << type))
        return;
    auto *out = &std::cout;
    if (type == MessageType::Warning || type == MessageType::Error)
        out = &std::cerr;
    if (colors_enabled && colors[type] != Color::Original)
        logger.set_color(colors[type]);
    (*out) << format(fmt, args);
    if (colors_enabled && colors[type] != Color::Original)
        logger.set_color(Color::Original);
}

Logger::Logger() : p(new Priv(*this))
{
    unmute();
}

Logger::~Logger()
{
}

void Logger::info(std::string fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    p->log(MessageType::Info, fmt, args);
    va_end(args);
}

void Logger::success(std::string fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    p->log(MessageType::Success, fmt, args);
    va_end(args);
}

void Logger::warn(std::string fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    p->log(MessageType::Warning, fmt, args);
    va_end(args);
}

void Logger::err(std::string fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    p->log(MessageType::Error, fmt, args);
    va_end(args);
}

void Logger::debug(std::string fmt, ...)
{
    std::va_list args;
    va_start(args, fmt);
    p->log(MessageType::Debug, fmt, args);
    va_end(args);
}

void Logger::flush()
{
    std::cout.flush();
    // stderr should be nonbuffered
}

void Logger::mute()
{
    p->muted = 0xFF;
}

void Logger::unmute()
{
    p->muted = 0;
}

void Logger::mute(MessageType type)
{
    p->muted |= 1 << type;
}

void Logger::unmute(MessageType type)
{
    p->muted &= ~(1 << type);
}

bool Logger::colors_enabled() const
{
    return p->colors_enabled;
}

void Logger::disable_colors()
{
    p->colors_enabled = false;
}

void Logger::enable_colors()
{
    p->colors_enabled = true;
}
