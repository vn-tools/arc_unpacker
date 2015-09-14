#pragma once

#include <string>
#include "err.h"
#include "arg_parser.h"

namespace au {
namespace util {

    template<typename T> struct PluginDefinition final
    {
        std::string name;
        std::string description;
        T value;
    };

    template<typename T> class PluginManager final
    {
    public:
        PluginManager()
        {
        }

        ~PluginManager()
        {
        }

        void add(
            const std::string &name,
            const std::string &description,
            T value)
        {
            PluginDefinition<T> d;
            d.name = name;
            d.description = description;
            d.value = value;
            definitions.push_back(d);
        }

        void register_cli_options(
            ArgParser &arg_parser, const std::string &description)
        {
            auto sw = arg_parser.register_switch({"-p", "--plugin"})
                ->set_value_name("PLUGIN")
                ->set_description(description);
            for (auto &def : definitions)
                sw->add_possible_value(def.name, def.description);
        }

        T get_from_string(const std::string &plugin)
        {
            for (const auto &definition : definitions)
                if (definition.name == plugin)
                    return definition.value;
            throw err::UsageError("Unrecognized plugin: " + plugin);
        }

        T get_from_cli_options(const ArgParser &arg_parser, bool mandatory)
        {
            if (!arg_parser.has_switch("plugin"))
            {
                if (mandatory)
                    throw err::UsageError("Plugin not specified");
                return nullptr;
            }
            return get_from_string(arg_parser.get_switch("plugin"));
        }

        std::vector<T> get_all() const
        {
            std::vector<T> ret;
            for (auto &def : definitions)
                ret.push_back(def.value);
            return ret;
        }

    private:
        std::vector<PluginDefinition<T>> definitions;
    };

} }
