#pragma once

#include "arg_parser.h"
#include "dec/nitroplus/npa_filter.h"

namespace au {
namespace dec {
namespace nitroplus {

    class NpaFilterRegistry final
    {
    public:
        NpaFilterRegistry();
        ~NpaFilterRegistry();
        void register_cli_options(ArgParser &arg_parser) const;
        void parse_cli_options(const ArgParser &arg_parser);
        std::shared_ptr<NpaFilter> get_filter() const;

    private:
        struct Priv;
        std::unique_ptr<Priv> p;
    };

} } }
