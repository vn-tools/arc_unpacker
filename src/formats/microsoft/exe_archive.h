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
            bool is_recognized_internal(File &) const override;
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
