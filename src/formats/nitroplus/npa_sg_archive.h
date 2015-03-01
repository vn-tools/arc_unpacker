#ifndef FORMATS_NITROPLUS_NPA_SG_ARCHIVE_H
#define FORMATS_NITROPLUS_NPA_SG_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Nitroplus
    {
        class NpaSgArchive final : public Archive
        {
        public:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
