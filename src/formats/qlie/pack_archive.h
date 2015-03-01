#ifndef FORMATS_QLIE_PACK_ARCHIVE_H
#define FORMATS_QLIE_PACK_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace QLiE
    {
        class PackArchive final : public Archive
        {
        public:
            PackArchive();
            ~PackArchive();
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
