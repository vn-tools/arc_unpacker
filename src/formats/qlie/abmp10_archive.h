#ifndef FORMATS_QLIE_ABMP10_ARCHIVE_H
#define FORMATS_QLIE_ABMP10_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace QLiE
    {
        class Abmp10Archive final : public Archive
        {
        public:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
