#ifndef FORMATS_TOUHOU_ANM_ARCHIVE_H
#define FORMATS_TOUHOU_ANM_ARCHIVE_H
#include "formats/archive.h"

namespace Formats
{
    namespace Touhou
    {
        class AnmArchive final : public Archive
        {
        protected:
            void unpack_internal(File &, FileSaver &) const override;
            FileNamingStrategy get_file_naming_strategy() const override;
        };
    }
}

#endif
