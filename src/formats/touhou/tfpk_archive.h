#ifndef FORMATS_TOUHOU_TFPK_ARCHIVE_H
#define FORMATS_TOUHOU_TFPK_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
        class TfpkArchive final : public Archive
        {
        public:
            TfpkArchive();
            ~TfpkArchive();
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
