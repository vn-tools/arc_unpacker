#pragma once

#include "arg_parser.h"
#include "fmt/kirikiri/xp3_filter.h"
#include "types.h"

namespace au {
namespace fmt {
namespace kirikiri {

    class Xp3FilterRegistry final
    {
    public:
        Xp3FilterRegistry();
        ~Xp3FilterRegistry();
        void register_cli_options(ArgParser &arg_parser) const;
        void parse_cli_options(const ArgParser &arg_parser);
        void set_decoder(Xp3Filter &filter);
    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
