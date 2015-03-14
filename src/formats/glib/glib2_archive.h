#ifndef FORMATS_GLIB_GLIB2_ARCHIVE_H
#define FORMATS_GLIB_GLIB2_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Glib
    {
        class Glib2Archive final : public Archive
        {
        public:
            void unpack_internal(File &, FileSaver &) const override;
        };
    }
}

#endif
