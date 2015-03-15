#ifndef FORMATS_LIARSOFT_LWG_ARCHIVE_H
#define FORMATS_LIARSOFT_LWG_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace LiarSoft
    {
        class LwgArchive final : public Archive
        {
        public:
            LwgArchive();
            ~LwgArchive();
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
