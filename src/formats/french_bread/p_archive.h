#ifndef FORMATS_FRENCH_BREAD_P_ARCHIVE_H
#define FORMATS_FRENCH_BREAD_P_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace FrenchBread
    {
        class PArchive final : public Archive
        {
        public:
            PArchive();
            ~PArchive();
            void add_cli_help(ArgParser &) const override;
            void parse_cli_options(ArgParser &) override;
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
