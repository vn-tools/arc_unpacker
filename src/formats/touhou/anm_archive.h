#ifndef FORMATS_TOUHOU_ANM_ARCHIVE_H
#define FORMATS_TOUHOU_ANM_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
        class AnmArchive final : public Archive
        {
        public:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
