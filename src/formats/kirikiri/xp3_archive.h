#ifndef FORMATS_KIRIKIRI_XP3_ARCHIVE_H
#define FORMATS_KIRIKIRI_XP3_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Kirikiri
    {
        class Xp3Archive final : public Archive
        {
        public:
            Xp3Archive();
            ~Xp3Archive();
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
