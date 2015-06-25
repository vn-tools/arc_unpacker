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
        protected:
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Priv;
            std::unique_ptr<Priv> p;
        };
    }
}

#endif
