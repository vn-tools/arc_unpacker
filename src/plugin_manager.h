#pragma once

#include <map>
#include <string>
#include "algo/any.h"
#include "arg_parser.h"
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
        BasePluginManager() : BasePluginManager("plugin") {}
        BasePluginManager(const std::string &option_name)
            : option_name(option_name) {}

        virtual ~BasePluginManager() {}

        inline void register_cli_options(
            ArgParser &parser, const std::string &description) const
        {
            auto sw = parser.register_switch({option_name})
                ->set_value_name("PLUGIN")
                ->set_description(description);
            for (const auto &def : definitions)
                sw->add_possible_value(def->name, def->description);
        }

        inline void parse_cli_options(const ArgParser &parser)
        {
            if (parser.has_switch(option_name))
                set(parser.get_switch(option_name));
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
            auto def = std::make_unique<PluginDefinition>();
            def->name = name;
            def->description = description;
            def->value = value;
            definitions.push_back(std::move(def));
        }

        inline algo::any &get_impl() const
        {
            if (definitions.empty())
                throw std::logic_error("No plugins were defined!");
            for (const auto &def : definitions)
                if (def->name == used_value_name)
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
            const algo::any &ret = get_impl();
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
