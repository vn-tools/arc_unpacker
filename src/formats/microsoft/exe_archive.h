#ifndef FORMATS_MICROSOFT_EXE_ARCHIVE_H
#define FORMATS_MICROSOFT_EXE_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Microsoft
    {
        class ExeArchive final : public Archive
        {
        protected:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
