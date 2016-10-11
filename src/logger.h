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

#pragma once

#include <memory>
#include <string>

namespace au {

    class Logger final
    {
    public:
        enum MessageType
        {
            Summary,
            Info,
            Success,
            Warning,
            Error,
            Debug,
        };

        enum class Color : unsigned char
        {
            Black,
            Navy,
            Green,
            Teal,
            Maroon,
            Purple,
            Brown,
            Grey,
            DarkGrey,
            Blue,
            Lime,
            Cyan,
            Red,
            Magenta,
            Yellow,
            White,

            Original
        };

        Logger();
        Logger(const Logger &other_logger);
        ~Logger();

        void set_color(const Color c);
        void set_prefix(const std::string &prefix);
        void log(const MessageType type, const std::string fmt, ...) const;
        void info(const std::string str, ...) const;
        void success(const std::string str, ...) const;
        void warn(const std::string str, ...) const;
        void err(const std::string str, ...) const;
        void debug(const std::string str, ...) const;
        void flush() const;

        void mute();
        void unmute();
        void mute(const MessageType type);
        void unmute(const MessageType type);

        bool colors_enabled() const;
        void disable_colors();
        void enable_colors();

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

}
