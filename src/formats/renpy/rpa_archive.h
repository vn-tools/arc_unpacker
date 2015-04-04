#ifndef FORMATS_RENPY_RPA_ARCHIVE_H
#define FORMATS_RENPY_RPA_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Renpy
    {
        class RpaArchive final : public Archive
        {
        protected:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
