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

#include <map>
#include <string>
#include "algo/any.h"
#include "arg_parser.h"
#include "arg_parser_decorator.h"
#include "err.h"

namespace au {

    struct PluginDefinition final
    {
        std::string name;
        std::string description;
        algo::any value;
    };

    class BasePluginManager
    {
    public:
        BasePluginManager() : BasePluginManager("--plugin") {}
        BasePluginManager(const std::string &option_name)
            : option_name(option_name) {}

        virtual ~BasePluginManager() {}

        inline ArgParserDecorator create_arg_parser_decorator(
            const std::string &description)
        {
            return ArgParserDecorator(
                [description, this](ArgParser &arg_parser)
                {
                    auto sw = arg_parser.register_switch({option_name})
                        ->set_value_name("PLUGIN")
                        ->set_description(description);
                    for (const auto &def : definitions)
                        sw->add_possible_value(def->name, def->description);
                },
                [this](const ArgParser &arg_parser)
                {
                    if (arg_parser.has_switch(option_name))
                        set(arg_parser.get_switch(option_name));
                });
        }

        inline bool is_set() const
        {
            return !used_value_name.empty();
        }

        inline void set(const std::string &name)
        {
            for (const auto &def : definitions)
            {
                if (def->name == name)
                {
                    used_value_name = name;
                    return;
                }
            }
            throw err::UsageError("Unrecognized plugin: " + name);
        }

    protected:
        inline void add_impl(
            const std::string &name,
            const std::string &description,
            const algo::any value)
        {
            for (const auto &def : definitions)
                if (def->name == name)
                    throw std::logic_error("Plugin " + name + " defined twice");
            auto def = std::make_unique<PluginDefinition>();
            def->name = name;
            def->description = description;
            def->value = value;
            definitions.push_back(std::move(def));
        }

        inline algo::any &get_impl(const std::string &check) const
        {
            if (definitions.empty())
                throw std::logic_error("No plugins were defined!");
            for (const auto &def : definitions)
                if (def->name == check)
                    return def->value;
            throw err::UsageError("No plugin was selected.");
        }

        std::string option_name;
        std::vector<std::unique_ptr<PluginDefinition>> definitions;
        std::string used_value_name;
    };

    template<typename T> class PluginManager final : public BasePluginManager
    {
    public:
        PluginManager() : BasePluginManager()
        {
        }

        PluginManager(const std::string &option_name)
            : BasePluginManager(option_name)
        {
        }

        void add(
            const std::string &name,
            const std::string &description,
            const T value)
        {
            add_impl(name, description, value);
        }

        inline T get() const
        {
            const algo::any &ret = get_impl(used_value_name);
            return ret.template get<T>();
        }

        inline T get(const std::string &name) const
        {
            const algo::any &ret = get_impl(name);
            return ret.template get<T>();
        }

        inline std::vector<T> get_all() const
        {
            std::vector<T> ret;
            for (const auto &def : definitions)
                ret.push_back(def->value.template get<T>());
            return ret;
        }
    };

}
