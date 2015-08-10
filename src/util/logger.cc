#include <iostream>
#include "util/logger.h"

using namespace au::util;

struct Logger::Priv
{
    bool muted = false;
};

Logger::Logger() : p(new Priv)
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

void Logger::err(const std::string &str)
{
    if (!p->muted)
        std::cerr << str;
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
