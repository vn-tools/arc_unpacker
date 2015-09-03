#pragma once

#include "fmt/kirikiri/xp3_filter.h"
#include "arg_parser.h"

namespace au {
namespace fmt {
namespace kirikiri {

    class Xp3FilterFactory final
    {
    public:
        Xp3FilterFactory();
        ~Xp3FilterFactory();
        void register_cli_options(ArgParser &arg_parser);
        std::unique_ptr<Xp3Filter> get_filter_from_cli_options(
            const ArgParser &arg_parser);
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
