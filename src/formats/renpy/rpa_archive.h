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
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
