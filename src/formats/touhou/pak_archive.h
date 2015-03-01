#ifndef FORMATS_TOUHOU_PAK_ARCHIVE_H
#define FORMATS_TOUHOU_PAK_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
        class PakArchive final : public Archive
        {
        public:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
