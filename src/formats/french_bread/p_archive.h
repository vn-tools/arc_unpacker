#ifndef FORMATS_FRENCH_BREAD_P_ARCHIVE_H
#define FORMATS_FRENCH_BREAD_P_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace FrenchBread
    {
        class PArchive final : public Archive
        {
        public:
            PArchive();
            ~PArchive();
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
