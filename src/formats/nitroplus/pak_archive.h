#ifndef FORMATS_NITROPLUS_PAK_ARCHIVE_H
#define FORMATS_NITROPLUS_PAK_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Nitroplus
    {
        class PakArchive final : public Archive
        {
        public:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
