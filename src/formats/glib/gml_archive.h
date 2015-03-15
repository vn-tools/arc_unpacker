#ifndef FORMATS_GLIB_GML_ARCHIVE_H
#define FORMATS_GLIB_GML_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Glib
    {
        class GmlArchive final : public Archive
        {
        public:
            GmlArchive();
            ~GmlArchive();
            void add_cli_help(ArgParser &arg_parser) const;
            void parse_cli_options(ArgParser &arg_parser);
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
