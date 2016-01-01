#pragma once

#include "arg_parser.h"
#include "dec/shiina_rio/warc/plugin.h"

namespace au {
namespace dec {
namespace shiina_rio {
namespace warc {

    class PluginRegistry final
    {
    public:
        PluginRegistry();
        ~PluginRegistry();
        void register_cli_options(ArgParser &arg_parser) const;
        void parse_cli_options(const ArgParser &arg_parser);
        std::shared_ptr<Plugin> get_plugin() const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } } }
