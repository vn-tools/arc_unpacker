#ifndef FORMATS_KIRIKIRI_XP3_FILTER_FACTORY_H
#define FORMATS_KIRIKIRI_XP3_FILTER_FACTORY_H
#include "formats/kirikiri/xp3_filter.h"
#include "arg_parser.h"

namespace Formats
{
    namespace Kirikiri
    {
        class Xp3FilterFactory final
        {
        public:
            Xp3FilterFactory();
            ~Xp3FilterFactory();
            void add_cli_help(ArgParser &arg_parser);
            std::unique_ptr<Xp3Filter> get_filter_from_cli_options(
                const ArgParser &arg_parser);
        private:
            struct Priv;
            std::unique_ptr<Priv> p;
        };
    }
}

#endif
