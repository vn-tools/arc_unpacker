#include "util/logger.h"
#include <iostream>

using namespace au::util;

struct Logger::Priv final
{
    Priv(Logger &logger);
    void try_set_color(Logger::Color color);

    bool muted = false;
    bool colors_enabled = true;
    Logger &logger;
};

Logger::Priv::Priv(Logger &logger) : logger(logger)
{
}

void Logger::Priv::try_set_color(Logger::Color color)
{
    if (colors_enabled)
        logger.set_color(color);
}

Logger::Logger() : p(new Priv(*this))
{
}

Logger::~Logger()
{
}

void Logger::info(const std::string &str)
{
    if (!p->muted)
        std::cout << str;
}

void Logger::success(const std::string &str)
{
    if (!p->muted)
    {
        p->try_set_color(Color::Lime);
        std::cout << str;
        p->try_set_color(Color::Original);
    }
}

void Logger::warn(const std::string &str)
{
    if (!p->muted)
    {
        p->try_set_color(Color::Yellow);
        std::cerr << str;
        p->try_set_color(Color::Original);
    }
}

void Logger::err(const std::string &str)
{
    if (!p->muted)
    {
        p->try_set_color(Color::Red);
        std::cerr << str;
        p->try_set_color(Color::Original);
    }
}

void Logger::flush()
{
    std::cout.flush();
    //stderr should be nonbuffered
}

void Logger::mute()
{
    p->muted = true;
}

void Logger::unmute()
{
    p->muted = false;
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
