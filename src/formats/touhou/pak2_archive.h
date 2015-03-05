#ifndef FORMATS_TOUHOU_PAK2_ARCHIVE_H
#define FORMATS_TOUHOU_PAK2_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
        class Pak2Archive final : public Archive
        {
        public:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
