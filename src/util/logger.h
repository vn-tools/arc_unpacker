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
        Debug,
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
        void info(std::string str, ...);
        void success(std::string str, ...);
        void warn(std::string str, ...);
        void err(std::string str, ...);
        void debug(std::string str, ...);
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
