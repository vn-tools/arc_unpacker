#pragma once

#include <memory>
#include <string>
#include <vector>

namespace au {

    class ArgParser final
    {
    public:
        ArgParser();
        ~ArgParser();

        void clear_help();
        void add_help(const std::string invocation, const std::string desc);
        void print_help() const;

        void parse(const std::vector<std::string> args);

        bool has_flag(const std::string argument) const;
        bool has_switch(const std::string key) const;

        const std::string get_switch(const std::string key) const;
        const std::vector<std::string> get_stray() const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

}
