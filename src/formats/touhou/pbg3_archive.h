#ifndef FORMATS_TOUHOU_PBG3_ARCHIVE_H
#define FORMATS_TOUHOU_PBG3_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
        class Pbg3Archive final : public Archive
        {
        public:
            Pbg3Archive();
            ~Pbg3Archive();
            void unpack_internal(File &, FileSaver &) const override;
        private:
            struct Internals;
            std::unique_ptr<Internals> internals;
        };
    }
}

#endif
