#ifndef FORMATS_MICROSOFT_EXE_ARCHIVE
#define FORMATS_MICROSOFT_EXE_ARCHIVE
#include "formats/archive.h"

namespace Formats
{
    namespace Microsoft
    {
        class ExeArchive final : public Archive
        {
        public:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
