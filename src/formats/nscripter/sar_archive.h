#ifndef FORMATS_NSCRIPTER_SAR_ARCHIVE_H
#define FORMATS_NSCRIPTER_SAR_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace NScripter
    {
        class SarArchive final : public Archive
        {
        public:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
