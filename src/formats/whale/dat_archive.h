#ifndef FORMATS_WHALE_DAT_ARCHIVE_H
#define FORMATS_WHALE_DAT_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Whale
    {
        class DatArchive final : public Archive
        {
        public:
            DatArchive();
            ~DatArchive();
            void add_cli_help(ArgParser &) const override;
            void parse_cli_options(const ArgParser &) override;
        protected:
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
