#ifndef FORMATS_NITROPLUS_NPA_ARCHIVE_H
#define FORMATS_NITROPLUS_NPA_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Nitroplus
    {
        class NpaArchive final : public Archive
        {
        public:
            NpaArchive();
            ~NpaArchive();
            void add_cli_help(ArgParser &) const override;
            void parse_cli_options(const ArgParser &) override;
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
