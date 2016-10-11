// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "logger.h"
#include <windows.h>

using namespace au;

static WORD get_win_color(const Logger::Color c)
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

    throw std::logic_error("Unknown color");
}

void Logger::set_color(const Logger::Color c)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, get_win_color(c));
}
