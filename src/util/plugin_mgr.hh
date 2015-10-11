#pragma once

#include <map>
#include <string>
#include "err.h"
#include "arg_parser.h"

namespace au {
namespace util {

    template<typename T> struct PluginDefinition
    {
        std::string name;
        std::string description;
        T value;
    };

    template<typename T> class PluginManager final
    {
    public:
        inline void add(
            const std::string &name, const std::string &description, T value)
        {
            auto def = std::make_unique<PluginDefinition<T>>();
            def->name = name;
            def->description = description;
            def->value = value;
            definitions.push_back(std::move(def));
        }

        inline void register_cli_options(
            ArgParser &parser, const std::string &description) const
        {
            auto sw = parser.register_switch({"-p", "--plugin"})
                ->set_value_name("PLUGIN")
                ->set_description(description);
            for (auto &def : definitions)
                sw->add_possible_value(def->name, def->description);
        }

        inline void parse_cli_options(const ArgParser &parser)
        {
            if (parser.has_switch("plugin"))
                set(parser.get_switch("plugin"));
        }

        inline bool is_set() const
        {
            return !used_value_name.empty();
        }

        inline void set(const std::string &name)
        {
            for (auto &def : definitions)
            {
                if (def->name == name)
                {
                    used_value_name = name;
                    return;
                }
            }
            throw err::UsageError("Unrecognized plugin: " + name);
        }

        inline T get() const
        {
            if (definitions.empty())
                throw std::logic_error("No plugins were defined!");
            for (auto &def : definitions)
                if (def->name == used_value_name)
                    return def->value;
            throw err::UsageError("No plugin was selected.");
        }

        inline std::vector<T> get_all() const
        {
            std::vector<T> ret;
            for (auto &def : definitions)
                ret.push_back(def->value);
            return ret;
        }

    private:
        std::vector<std::unique_ptr<PluginDefinition<T>>> definitions;
        std::string used_value_name;
    };

} }
