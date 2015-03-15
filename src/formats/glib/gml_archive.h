#ifndef FORMATS_GLIB_GML_ARCHIVE_H
#define FORMATS_GLIB_GML_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Glib
    {
        class GmlArchive final : public Archive
        {
        public:
            GmlArchive();
            ~GmlArchive();
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
