#ifndef FORMATS_MINATOSOFT_PAC_ARCHIVE_H
#define FORMATS_MINATOSOFT_PAC_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace MinatoSoft
    {
        class PacArchive final : public Archive
        {
        public:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
