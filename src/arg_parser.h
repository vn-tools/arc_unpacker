#pragma once

#include <functional>
#include <memory>
#include <vector>
#include "logger.h"

namespace au {

    struct Option
    {
        virtual Option *set_description(const std::string &desc) = 0;
    };

    struct Switch : Option
    {
        virtual Switch *set_description(const std::string &desc) = 0;
        virtual Switch *set_value_name(const std::string &name) = 0;
        virtual Switch *add_possible_value(
            const std::string &value, const std::string &description = "") = 0;
        virtual Switch *hide_possible_values() = 0;
    };

    struct Flag : Option
    {
        virtual Flag *set_description(const std::string &desc) = 0;
    };

    class ArgParser final
    {
    private:
        using NameList = const std::initializer_list<std::string>;

    public:
        ArgParser();
        ~ArgParser();

        void print_help(const Logger &logger) const;

        Flag *register_flag(NameList &names);
        Switch *register_switch(NameList &names);

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
