#include <windows.h>
#include "util/logger.h"

using namespace au::util;

static WORD get_win_color(Logger::Color c)
{
    if (c == Logger::Color::Black)
            return 0;

    if (c == Logger::Color::Navy)
        return FOREGROUND_BLUE;

    if (c == Logger::Color::Green)
        return FOREGROUND_GREEN;

    if (c == Logger::Color::Teal)
        return FOREGROUND_GREEN | FOREGROUND_BLUE;

    if (c == Logger::Color::Maroon)
        return FOREGROUND_RED;

    if (c == Logger::Color::Purple)
        return FOREGROUND_RED | FOREGROUND_BLUE;

    if (c == Logger::Color::Brown)
        return FOREGROUND_RED | FOREGROUND_GREEN;

    if (c == Logger::Color::Grey || c == Logger::Color::DarkGrey)
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

    if (c == Logger::Color::Blue)
        return FOREGROUND_INTENSITY | FOREGROUND_BLUE;

    if (c == Logger::Color::Lime)
        return FOREGROUND_INTENSITY | FOREGROUND_GREEN;

    if (c == Logger::Color::Cyan)
        return FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE;

    if (c == Logger::Color::Red)
        return FOREGROUND_INTENSITY | FOREGROUND_RED;

    if (c == Logger::Color::Magenta)
        return FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE;

    if (c == Logger::Color::Yellow)
        return FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;

    if (c == Logger::Color::White || c == Logger::Color::Original)
    {
        return FOREGROUND_INTENSITY
            | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }

    throw std::runtime_error("Unknown color");
}

void Logger::set_color(Logger::Color c)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, get_win_color(c));
}
