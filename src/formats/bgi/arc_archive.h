#ifndef FORMATS_BGI_ARC_ARCHIVE_H
#define FORMATS_BGI_ARC_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Bgi
    {
        class ArcArchive final : public Archive
        {
        public:
            ArcArchive();
            ~ArcArchive();
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
