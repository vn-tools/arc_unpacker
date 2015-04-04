#ifndef FORMATS_NITROPLUS_PAK_ARCHIVE_H
#define FORMATS_NITROPLUS_PAK_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Nitroplus
    {
        class PakArchive final : public Archive
        {
        protected:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
