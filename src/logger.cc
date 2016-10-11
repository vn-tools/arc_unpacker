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
#include <cstdarg>
#include <iostream>
#include <mutex>
#include "algo/format.h"
#include "algo/str.h"

using namespace au;

static std::mutex mutex;

struct Logger::Priv final
{
    Priv(Logger &logger);
    void log(
        const MessageType type, const std::string fmt, std::va_list args) const;

    Logger &logger;
    Color colors[6];
    int muted = 0;
    bool colors_enabled = true;
    std::string prefix;
};

Logger::Priv::Priv(Logger &logger) : logger(logger)
{
    colors[MessageType::Summary] = Color::Original;
    colors[MessageType::Info] = Color::Original;
    colors[MessageType::Success] = Color::Lime;
    colors[MessageType::Warning] = Color::Yellow;
    colors[MessageType::Error] = Color::Red;
    colors[MessageType::Debug] = Color::Cyan;
}

void Logger::Priv::log(
    const MessageType type, const std::string fmt, std::va_list args) const
{
    std::unique_lock<std::mutex> lock(mutex);
    if (muted & (1 << type))
        return;
    auto *out = &std::cout;
    if (type == MessageType::Warning || type == MessageType::Error)
        out = &std::cerr;
    const auto output = algo::format(fmt, args);
    for (const auto line : algo::split(output, '\n', true))
    {
        (*out) << prefix;
        if (colors_enabled && colors[type] != Color::Original)
            logger.set_color(colors[type]);
        (*out) << line;
        if (colors_enabled && colors[type] != Color::Original)
            logger.set_color(Color::Original);
    }
}

Logger::Logger(const Logger &other_logger) : p(new Priv(*this))
{
    p->muted = other_logger.p->muted;
    p->colors_enabled = other_logger.p->colors_enabled;
    p->prefix = other_logger.p->prefix;
}

Logger::Logger() : p(new Priv(*this))
{
    unmute();
}

Logger::~Logger()
{
}

void Logger::set_prefix(const std::string &prefix)
{
    p->prefix = prefix;
}

void Logger::log(
    const MessageType message_type, const std::string fmt, ...) const
{
    std::va_list args;
    va_start(args, fmt);
    p->log(message_type, fmt, args);
    va_end(args);
}

void Logger::info(const std::string fmt, ...) const
{
    std::va_list args;
    va_start(args, fmt);
    p->log(MessageType::Info, fmt, args);
    va_end(args);
}

void Logger::success(const std::string fmt, ...) const
{
    std::va_list args;
    va_start(args, fmt);
    p->log(MessageType::Success, fmt, args);
    va_end(args);
}

void Logger::warn(const std::string fmt, ...) const
{
    std::va_list args;
    va_start(args, fmt);
    p->log(MessageType::Warning, fmt, args);
    va_end(args);
}

void Logger::err(const std::string fmt, ...) const
{
    std::va_list args;
    va_start(args, fmt);
    p->log(MessageType::Error, fmt, args);
    va_end(args);
}

void Logger::debug(const std::string fmt, ...) const
{
    std::va_list args;
    va_start(args, fmt);
    p->log(MessageType::Debug, fmt, args);
    va_end(args);
}

void Logger::flush() const
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

void Logger::mute(const MessageType type)
{
    p->muted |= 1 << type;
}

void Logger::unmute(const MessageType type)
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
