#include "util/logger.h"
#include <iostream>
#include <unistd.h>
#include <stdio.h>

using namespace au::util;

static std::string get_ansi_color(Logger::Color c)
{
    // switch with strongly typed enums crashes MinGW -O3 builds
    if (c == Logger::Color::Black)    return "\033[22;30m";
    else if (c == Logger::Color::Navy)     return "\033[22;34m";
    else if (c == Logger::Color::Green)    return "\033[22;32m";
    else if (c == Logger::Color::Teal)     return "\033[22;36m";
    else if (c == Logger::Color::Maroon)   return "\033[22;31m";
    else if (c == Logger::Color::Purple)   return "\033[22;35m";
    else if (c == Logger::Color::Brown)    return "\033[22;33m";
    else if (c == Logger::Color::Grey)     return "\033[22;37m";
    else if (c == Logger::Color::DarkGrey) return "\033[01;30m";
    else if (c == Logger::Color::Blue)     return "\033[01;34m";
    else if (c == Logger::Color::Lime)     return "\033[01;32m";
    else if (c == Logger::Color::Cyan)     return "\033[01;36m";
    else if (c == Logger::Color::Red)      return "\033[01;31m";
    else if (c == Logger::Color::Magenta)  return "\033[01;35m";
    else if (c == Logger::Color::Yellow)   return "\033[01;33m";
    else if (c == Logger::Color::White)    return "\033[01;37m";
    else if (c == Logger::Color::Original) return "\033[m";
    return "";
}

void Logger::set_color(Logger::Color c)
{
    if (isatty(STDIN_FILENO))
        std::cout << get_ansi_color(c);
    if (isatty(STDERR_FILENO))
        std::cerr << get_ansi_color(c);
}
