#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace au {

    class ArgParser final
    {
    public:
        ArgParser();
        ~ArgParser();

        void print_help() const;

        void register_flag(
            const std::initializer_list<std::string> &names,
            const std::string &description);

        void register_switch(
            const std::initializer_list<std::string> &names,
            const std::string &value_name,
            const std::string &description);

        void parse(const std::vector<std::string> &args);

        bool has_flag(const std::string &name) const;
        bool has_switch(const std::string &name) const;

        const std::string get_switch(const std::string &name) const;
        const std::vector<std::string> get_stray() const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

}
