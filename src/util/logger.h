#pragma once

#include <memory>
#include <string>

namespace au {
namespace util {

    enum MessageType
    {
        Info,
        Success,
        Warning,
        Error,
    };

    class Logger final
    {
    public:
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

        void set_color(Color c);
        void info(const std::string &str);
        void success(const std::string &str);
        void warn(const std::string &str);
        void err(const std::string &str);
        void flush();

        void mute();
        void unmute();
        void mute(MessageType type);
        void unmute(MessageType type);

        bool colors_enabled() const;
        void disable_colors();
        void enable_colors();

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }
