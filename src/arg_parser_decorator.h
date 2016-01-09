#pragma once

#include "arg_parser.h"

namespace au {

    class ArgParserDecorator final
    {
    public:
        ArgParserDecorator(
            const std::function<void(ArgParser &)> register_callback,
            const std::function<void(const ArgParser &)> parse_callback);

        void register_cli_options(ArgParser &arg_parser) const;

        void parse_cli_options(const ArgParser &arg_parser) const;

    private:
        std::function<void(ArgParser &)> register_callback;
        std::function<void(const ArgParser &)> parse_callback;
    };

}
