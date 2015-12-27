#pragma once

#include <memory>
#include <string>

namespace au {

    class Logger final
    {
    public:
        enum MessageType
        {
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
        ~Logger();

        void set_color(const Color c);
        void set_prefix(const std::string &prefix);
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
