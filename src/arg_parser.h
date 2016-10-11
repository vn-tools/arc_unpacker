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

#include <functional>
#include <memory>
#include <vector>
#include "logger.h"

namespace au {

    struct Option
    {
        virtual ~Option() {}
        virtual Option *set_description(const std::string &desc) = 0;
    };

    struct Switch : Option
    {
        virtual ~Switch() {}
        virtual Switch *set_description(const std::string &desc) = 0;
        virtual Switch *set_value_name(const std::string &name) = 0;
        virtual Switch *add_possible_value(
            const std::string &value, const std::string &description = "") = 0;
        virtual Switch *hide_possible_values() = 0;
    };

    struct Flag : Option
    {
        virtual ~Flag() {}
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
