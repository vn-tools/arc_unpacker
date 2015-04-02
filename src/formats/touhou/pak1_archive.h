#ifndef FORMATS_TOUHOU_PAK1_ARCHIVE_H
#define FORMATS_TOUHOU_PAK1_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
        class Pak1Archive final : public Archive
        {
        public:
            Pak1Archive();
            ~Pak1Archive();
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
