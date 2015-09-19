#include "util/logger.h"
#include <iostream>

using namespace au::util;

struct Logger::Priv final
{
    Priv(Logger &logger);
    void log(MessageType type, const std::string &str);

    Logger &logger;
    Color colors[4];
    int muted = 0;
    bool colors_enabled = true;
};

Logger::Priv::Priv(Logger &logger) : logger(logger)
{
    colors[MessageType::Info] = Color::Original;
    colors[MessageType::Success] = Color::Lime;
    colors[MessageType::Warning] = Color::Yellow;
    colors[MessageType::Error] = Color::Red;
}

void Logger::Priv::log(MessageType type, const std::string &str)
{
    if (muted & (1 << type))
        return;
    auto *out = &std::cout;
    if (type != MessageType::Info && type != MessageType::Success)
        out = &std::cerr;
    if (colors_enabled && colors[type] != Color::Original)
        logger.set_color(colors[type]);
    (*out) << str;
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

void Logger::info(const std::string &str)
{
    p->log(MessageType::Info, str);
}

void Logger::success(const std::string &str)
{
    p->log(MessageType::Success, str);
}

void Logger::warn(const std::string &str)
{
    p->log(MessageType::Warning, str);
}

void Logger::err(const std::string &str)
{
    p->log(MessageType::Error, str);
}

void Logger::flush()
{
    std::cout.flush();
    //stderr should be nonbuffered
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
