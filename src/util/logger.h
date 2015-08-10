#ifndef AU_UTIL_LOG_H
#define AU_UTIL_LOG_H
#include <memory>
#include <string>

namespace au {
namespace util {

    class Logger
    {
    public:
        Logger();
        ~Logger();
        void info(const std::string &str);
        void err(const std::string &str);
        void flush();
        void mute();
        void unmute();
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} }

#endif
